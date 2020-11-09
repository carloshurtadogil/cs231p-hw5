#include <stdio.h> //remove if not using.

#include "util.h"//implementing

/**
 * Tranpose a given matrix
 * 
 * @param mat Matrix to be transposed
*/
void transpose_mat(Mat *mat) {
    int n = mat->n;
    double temp = -1.0;
    for (int i = 0; i < n ; i++) {
        for (int j = i + 1; j < n ; j++) {
            temp = mat->ptr[i * n + j];
            mat->ptr[i * n + j] = mat->ptr[j * n + i];
            mat->ptr[j * n + i] = temp;

        }
    }
}

/**
 * Use a single thread to transpose a matrix
 * 
 * @param mat Matrix to be tranposed
*/
void mat_sq_trans_st(Mat *mat){
    transpose_mat(mat);
    return;
}

void mat_sq_trans_mt(Mat *mat, unsigned int grain, unsigned int threads){
    //Put your code here.
    //example_helper_function(2000);
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