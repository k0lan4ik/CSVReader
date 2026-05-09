#ifndef CSVTABLE_H
#define CSVTABLE_H

typedef enum {
    CELL_RAW = 0,
    CELL_CALCULATING,
    CELL_DONE,
    CELL_ERROR
} CellState;

typedef struct {
    char* raw;      
    int value;       
    CellState state; 
} Cell;

typedef struct {
    char* text_buffer;
    Cell* cells;       
    int rows;
    int cols;
} Table;


Cell *table_get_cell(Table *t, int r, int c);
Cell *table_get_by_value(Table *t, char *val, int len);
void table_free(Table *t);

#endif
