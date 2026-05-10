#include "csvtable.h"
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

Cell* table_get_cell(Table *t, int r, int c) {
    if (r < 0 || r >= t->rows || c < 0 || c >= t->cols) {
        return NULL; 
    }
    return &t->cells[r * t->cols + c];
}

int table_get_by_value(Table *t, char *val, int len, int *r, int *c) {
    int i,j;
    
    for(int k = len - 1; k > 0; k--) {
        char found = 0;
        for(i = 0; i < t->cols; i++)
        {
            if(!strncmp(t->cells[i].raw, val, k)){
                found = 1;
                break;
            }
        }
        if(found) {
            for(j = 0; j < t->rows; j++)
            {
                if(!strncmp(t->cells[j * t->cols].raw, &val[k], len - k)){
                    *r = j;
                    *c = i;
                    return 1; 
                }
            }
        }
    }
    return 0; 
}

void table_free(Table *t) {
    if (t->cells) free(t->cells);
    if (t->text_buffer) free(t->text_buffer);
}