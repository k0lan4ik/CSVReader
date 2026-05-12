#include "mathparse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_number(int *value, const char *arg, const int len){
    char *endptr;
    *value = strtol(arg, &endptr, 10);
    if (endptr - arg != len) {
        return 0;
    }
    return 1;
}

int evaluate_cell(Table* t, int r, int c, int *value) {
    Cell* cell = table_get_cell(t, r, c);
    if (!cell || !cell->raw) {*value = 0; return 0;}
    if (cell->state == CELL_DONE) {*value = cell->value; return 0;}
    if (cell->state == CELL_CALCULATING) {return -1;}
    if (cell->state == CELL_ERROR) {return -2;}
    if (cell->state == CELL_DIV0) {return -3;}

    cell->state = CELL_CALCULATING;

    int arg1_len = 0, arg2_len = 0;
    char op = '\0', *arg1_s, *arg2_s, *exp = cell->raw;
    StateMath state = STMATH_START;
    for (int index = 0; exp[index]; index++)
    {  
        char last = exp[index];
        switch (state)
        {
        case STMATH_START:
            if(last != '=') state = STMATH_ERROR;
            else {state = STMATH_ARG1; arg1_s = &exp[index + 1];}
            break;
        case STMATH_ARG1:
            if((arg1_len > 0 && (last == '+' || last == '-')) || (last == '*' || last == '/')) {state = STMATH_OP; op = last;}
            else arg1_len++; 
            break;
        case STMATH_OP:
            state = STMATH_ARG2; 
            arg2_s = &exp[index]; 
            arg2_len++;
            break;
        case STMATH_ARG2:
            if(last == '+' || last == '-' || last == '*' || last == '/') {state = STMATH_ERROR;}
            else arg2_len++; 
            break;
        case STMATH_ERROR:
        break;
        }
    }
    if(state != STMATH_ARG2 && state != STMATH_ARG1){
        if(!check_number(&cell->value,cell->raw,strlen(cell->raw))){
               
            cell->state = CELL_ERROR; 
            return -2; 
        }
        *value = cell->value;
        cell->state = CELL_DONE; 
        return 0; 
    }

    int arg1 = 0, arg2 = 0; 
    if(!check_number(&arg1, arg1_s, arg1_len)) {
        int row, col;
        if(!table_get_by_value(t, arg1_s, arg1_len, &row, &col)){
           cell->state = CELL_ERROR; 
           return -2;
        }
       
        int err = evaluate_cell(t, row, col, &arg1);
        if(err == -1) return -1;
        else if(err == -2) {cell->state = CELL_ERROR; return -2;}
        
    }
    if(op != 0 && !check_number(&arg2, arg2_s, arg2_len)) {
       
        int row, col;
        if(!table_get_by_value(t, arg2_s, arg2_len, &row, &col)){
           cell->state = CELL_ERROR; 
           return -2;
        }
        int err = evaluate_cell(t, row, col, &arg2);
        if(err == -1) return -1;
        else if(err == -2) {cell->state = CELL_ERROR; return -2;}
    }

    switch (op)
    {
    case '+':
        cell->value = arg1 + arg2;
        break;
    case '-':
        cell->value = arg1 - arg2;
        break;
    case '*':
        cell->value = arg1 * arg2;
        break;
    case '/':
        if(arg2 == 0)
        {
            cell->state = CELL_DIV0;
            return -3;
        }
        cell->value = arg1 / arg2;
        break;
    case '\0':
        cell->value = arg1;
        break;
    }

    cell->state = CELL_DONE;
    *value = cell->value;
    return 0;
}

