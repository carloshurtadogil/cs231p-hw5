#include <stdio.h> //remove if not using.
#include <pthread.h>
#include "util.h"//implementing

typedef struct {
    Mat *X;
} Matrices;

/* GLOBAL VARIABLES */
pthread_mutex_t lock;
unsigned int N, max_row;
unsigned int batch_size = 0, current_row = 0;

/**
 * Tranpose a given matrix
 * 
 * @param mat Matrix to be transposed
 * @param start Starting index
 * @param end End index
*/
void transpose_mat(Mat *mat, int start, int end) {
    //printf("start: %d, end: %d\n", start, end);
    int n = max_row;
    double temp = -1.0;
    for (int i = start; i < end; i++) { // rows
        for (int j = i + 1; j < n ; j++) { // cols
            temp = mat->ptr[i * n + j];
            mat->ptr[i * n + j] = mat->ptr[j * n + i];
            mat->ptr[j * n + i] = temp;
        }
    }
}

/**
 * Grab a set of rows that have yet to be calculated
 * 
 * @param rows Rows to be calculated
 * @param size The amount of rows to calculate
*/
size_t get_calc_set(unsigned int rows[], size_t size) {
    size_t i = 0;
    int start_row;
    pthread_mutex_lock(&lock);

    if( current_row < max_row ) { // there remain rows unchecked
        start_row = current_row; // Start with current index
        current_row += batch_size;
        pthread_mutex_unlock(&lock);
        // Get the requested batch size or as many as legal
        for (i = 0; i < size && start_row <= max_row; ++i) {
            rows[i] = start_row++;
        }
    } else {
        pthread_mutex_unlock(&lock);
    }

    return i;
}

/**
 * Consumes BATCH_SIZE consecutive rows and calculate the product at each column
 * 
 * @param arg The Matrices struct with matrices A, B, and C to be calculated
*/
void *consume_and_calculate(void* arg) {
    Matrices *matrices = (Matrices *) arg;
    Mat *matrix = matrices->X;
    unsigned int set[batch_size];
    size_t actual_size = 0;
    // while there are rows left to be processed
    while ((actual_size = get_calc_set(set, batch_size)) > 0) {
        for (int i = 0; i < actual_size; ++i) {
            unsigned int curr_row = set[i];
            transpose_mat(matrix, curr_row, curr_row + 1);
        }

        // last batch has been completed
        if (actual_size < batch_size)
            break;
    }

    return NULL;
}

/**
 * Use a single thread to transpose a matrix
 * 
 * @param mat Matrix to be tranposed
*/
void mat_sq_trans_st(Mat *mat){
    max_row = mat->n;
    transpose_mat(mat, 0, max_row);
    return;
}

void mat_sq_trans_mt(Mat *mat, unsigned int grain, unsigned int threads){
    max_row = mat->n;
    batch_size = max_row/grain;
    Matrices matrices = { .X = mat };
    if (pthread_mutex_init(&lock, NULL) != 0)
       pthread_exit((void *)1);

    /*Create 'threads' amount of pthreads to carry out tasks*/
    pthread_t thread_arr[threads];
    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_arr[i], NULL, &consume_and_calculate, (void *) &matrices) != 0)
            pthread_exit((void *)1);
    }
        
    for (int i = 0; i < threads; ++i)
        pthread_join(thread_arr[i], NULL);

    pthread_mutex_destroy(&lock);
    return;
}

/*static void example_helper_function(int n){
    // Functions defined with the modifier 'static' are only visible
    // to other functions in this file. They cannot be called from
    // outside (for example, from main.c). Use them to organize your
    // code. Remember that in C, you cannot use a function until you
    // declare it, so declare all your utility functions above the
    // point where you use them.
    //
    // Maintain the mat_sq_trans_xt functions as lean as possible
    // because we are measuring their speed. Unless you are debugging,
    // do not print anything on them, that consumes precious time.
    //
    // You may delete this example helper function and structure, and
    // write your own useful ones.

    Example_Structure es1;
    es1.example = 13;
    es1.e_g = 7;
    printf("n = %d\n", es1.example + es1.e_g + n);
    return;
}*/