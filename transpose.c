#include <stdio.h> //remove if not using.
#include <pthread.h>
#include <stdlib.h>
#include "util.h"//implementing

typedef struct {
    Mat *X;
} Matrices;

/* GLOBAL VARIABLES */
pthread_mutex_t lock, malloc_lock;
unsigned int max_row;
unsigned int batch_size = 0, current_row = 0;
unsigned int c = 0;
unsigned int *i_indices = NULL;
unsigned int *j_indices = NULL;
unsigned int last_idx = 0;
unsigned int upper_t_size = 0;

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

/**
 * Tranpose a certain section of a matrix
 * 
 * @param mat Matrix to be tranposed
 * @param i Row index to be transposed
 * @param j Column index to be transposed
*/
void transpose(Mat *mat, unsigned int i, unsigned int j) {
    unsigned int n = mat->n;
    double temp = mat->ptr[i * n + j];
    mat->ptr[i * n + j] = mat->ptr[j * n + i];
    mat->ptr[j * n + i] = temp;
}

/**
 * Read in rows and columns to be tranposed
 * 
 * @param mat Matrix to be tranposed
 * @param is Rows to be processed
 * @param js Columns to be processed
 * @param len Coarseness for transposition
*/
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

/**
 * Retreive a certain amount of indices of the matrix to be transposed
 * 
 * @param is Indices representing rows
 * @param js Indices representing columns
*/
int get_indices(unsigned int is[], unsigned int js[]) {
    unsigned int cur;
    int i = 0;
    pthread_mutex_lock(&lock);
    if (last_idx < upper_t_size) {
        // still work to be done
        cur = last_idx;
        last_idx += c;
        pthread_mutex_unlock(&lock);
        for (i = 0; i < c && cur < upper_t_size; ++i, ++cur) {
            is[i] = i_indices[cur];
            js[i] = j_indices[cur];
        }
    } else
        pthread_mutex_unlock(&lock);
    return i;
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

    pthread_mutex_lock(&malloc_lock);
    unsigned int* is = malloc(sizeof(unsigned int) * c);
    unsigned int* js = malloc(sizeof(unsigned int) * c);
    pthread_mutex_unlock(&malloc_lock);
    // while there are rows left to be processed
    while ((actual_size = get_indices(is, js)) > 0) {
        transpose_part(mat, is, js, c);
        // last batch has been completed
        if (actual_size < c)
            break;
    }

    pthread_mutex_lock(&malloc_lock);
    free(is);
    free(js);
    pthread_mutex_unlock(&malloc_lock);
    return NULL;
}

/**
 * Use a single thread to transpose a matrix
 * 
 * @param mat Matrix to be transposed
*/
void mat_sq_trans_st(Mat *mat) {
    max_row = mat->n;
    transpose_mat(mat, 0, max_row);
}

/**
 * Initialize two pointer arrays to represent the necessary rows and columns to transpose
 * @param n size of arrays
*/
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

/**
 * Transpose a matrix with respect to multiple threads and different grains
 * 
 * @param mat Matrix to be transposed
 * @param grain Coarseness
 * @param threads Amount of threads to be utilized
*/
void mat_sq_trans_mt(Mat *mat, unsigned int grain, unsigned int threads) {
    i_indices = (unsigned int *) malloc(sizeof(unsigned int) * (mat->n * mat->n - mat->n) / 2);
    j_indices = (unsigned int *) malloc(sizeof(unsigned int) * (mat->n * mat->n - mat->n) / 2);
    generate_indices(mat->n);
    upper_t_size = (mat->n * mat->n - mat->n) / 2;
    last_idx = 0;

    max_row = mat->n;
    c = grain;
    batch_size = max_row / grain;
    current_row = 0;
    Matrices matrices = {.X = mat};
    if (pthread_mutex_init(&lock, NULL) != 0)
        pthread_exit((void *) 1);

    if (pthread_mutex_init(&malloc_lock, NULL) != 0)
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
    pthread_mutex_destroy(&malloc_lock);
    free(j_indices);
    free(i_indices);
}
