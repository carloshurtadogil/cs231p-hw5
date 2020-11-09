#include <stdio.h> //remove if not using.

#include "util.h"//implementing

typedef struct { //example structure
    int example;
    int e_g;
} Example_Structure;

static void example_helper_function(int n){
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
}

void mat_sq_trans_st(Mat *mat){
    //Put your code here.
    // example_helper_function(1000);
    return;
}

void mat_sq_trans_mt(Mat *mat, unsigned int grain, unsigned int threads){
    //Put your code here.
    //example_helper_function(2000);
    return;
}
