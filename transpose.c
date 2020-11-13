#include <stdio.h> //remove if not using.
#include <pthread.h>
#include <stdlib.h>
#include "util.h"//implementing

typedef struct {
    Mat *X;
} Matrices;

/* GLOBAL VARIABLES */
pthread_mutex_t lock;
unsigned int max_row;
unsigned int batch_size = 0, current_row = 0;
unsigned int c = 0;
unsigned int *i_indices = NULL;
unsigned int *j_indices = NULL;
unsigned int last_idx = 0;

/**
 * Transpose a given matrix
 * 
 * @param mat Matrix to be transposed
 * @param start Starting index
 * @param end End index
*/
void transpose_mat(Mat *mat, unsigned int start, unsigned int end) {
    //printf("start: %d, end: %d\n", start, end);
    unsigned int n = mat->n;
    double temp;
    for (unsigned int i = start; i < end; i++) { // row in original
        for (unsigned int j = i + 1; j < n; j++) { // index within row in original
            temp = mat->ptr[i * n + j];
            mat->ptr[i * n + j] = mat->ptr[j * n + i];
            mat->ptr[j * n + i] = temp;
        }
    }
}

void transpose(Mat *mat, unsigned int i, unsigned int j) {
    unsigned int n = mat->n;
    double temp = mat->ptr[i * n + j];
    mat->ptr[i * n + j] = mat->ptr[j * n + i];
    mat->ptr[j * n + i] = temp;
}

void transpose_part(Mat *mat, unsigned int is[], unsigned int js[], unsigned int len) {
    for (int x = 0; x < len; ++x) {
        transpose(mat, is[x], js[x]);
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
    unsigned int start_row;
    pthread_mutex_lock(&lock);

    if (current_row < max_row) { // there remain rows unchecked
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

int get_indices(unsigned int is[], unsigned int js[]) {
    int iterations = 0;
    pthread_mutex_lock(&lock);
    for (unsigned int i = last_idx; i < last_idx + c && last_idx + c < max_row; ++i, ++last_idx) {
        printf("calculating\n");
        is[i] = i_indices[i];
        js[i] = j_indices[i];
        ++iterations;
    }
    pthread_mutex_unlock(&lock);
    return iterations;
}

/**
 * Consumes BATCH_SIZE consecutive rows and calculate the product at each column
 * 
 * @param arg The Matrices struct with matrices A, B, and C to be calculated
*/
void *consume_and_calculate(void *arg) {
    Matrices *matrices = (Matrices *) arg;
    Mat *mat = matrices->X;
    int actual_size;

    unsigned int is[c];
    unsigned int js[c];

    // while there are rows left to be processed
    while ((actual_size = get_indices(is, js)) > 0) {
//        printf("Got %d indices", actual_size);
        transpose_part(mat, is, js, c);
        // last batch has been completed
        if (actual_size < c)
            break;
    }
    return NULL;
}

/**
 * Use a single thread to transpose a matrix
 * 
 * @param mat Matrix to be transposed
*/
void mat_sq_trans_st(Mat *mat) {
//    printf("\nAddress: %p\n\n", (void*) mat);
    max_row = mat->n;
    transpose_mat(mat, 0, max_row);
}

void generate_indices(unsigned int n) {
    int count = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            i_indices[count] = i;
            j_indices[count] = j;
            ++count;
        }
    }
}

void mat_sq_trans_mt(Mat *mat, unsigned int grain, unsigned int threads) {
//    printf("\nAddress: %p\n\n", (void*) mat);
    //mat_print(mat);
    i_indices = (unsigned int *) malloc(sizeof(unsigned int) * (mat->n * mat->n - mat->n) / 2);
    j_indices = (unsigned int *) malloc(sizeof(unsigned int) * (mat->n * mat->n - mat->n) / 2);
    generate_indices(mat->n);
//    for (int i = 0; i < 45; ++i){
//        printf("%d, ",i_indices[i]);
//    }
//    printf("\n");
//    for (int i = 0; i < 45; ++i){
//        printf("%d, ",j_indices[i]);
//    }
//    printf("\n");
    max_row = mat->n;
    c = grain;
    batch_size = max_row / grain;
    current_row = 0;
    Matrices matrices = {.X = mat};
    if (pthread_mutex_init(&lock, NULL) != 0)
        pthread_exit((void *) 1);

    /*Create 'threads' amount of pthreads to carry out tasks*/
    pthread_t thread_arr[threads];
    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_arr[i], NULL, &consume_and_calculate, (void *) &matrices) != 0)
            pthread_exit((void *) 1);
    }

    for (int i = 0; i < threads; ++i)
        pthread_join(thread_arr[i], NULL);

    pthread_mutex_destroy(&lock);
    free(j_indices);
    free(i_indices);
    //printf("after: \n");
    //mat_print(mat);
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