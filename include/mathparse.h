#ifndef MATHPARSER_H
#define MATHPARSER_H
#include "csvtable.h"

int evaluate_cell(Table* t, int r, int c, int *value);


typedef enum {
    STMATH_START = 0,
    STMATH_ARG1,
    STMATH_OP,
    STMATH_ARG2,
    STMATH_ERROR = -1,
}StateMath;


#endif