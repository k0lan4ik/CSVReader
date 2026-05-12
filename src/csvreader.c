#include "csvreader.h"
#include <stdio.h>
#include <stdlib.h>

// длинна с 0
// state 0 -> начало
// state 1 -> нет кавычек/ждём запятую или перенос строки
// state 2 -> в начале была кавычка, ждём закрывающую и потом в 0 
// state 3 -> ждём 2-ю кавычку или конец
// state 4 -> конец
// state -1 -> ошибочное состояние  
long len_cell(char *cell_start){
    StateParsing state = STPARS_START;
    long index = 0;
    for(; cell_start[index]; index++)
    {
        char symb = cell_start[index];
        switch (state)
        {
        case STPARS_START: {
            if(symb == '"') state = STPARS_QUOTE;
            else if(symb == SEPARATOR || symb == '\n' || symb == '\0') state = STPARS_END;
            else state = STPARS_NOQUOTE;
            break;
        }
        case STPARS_NOQUOTE: {    
            if(symb == '"') state = STPARS_ERROR;
            else if(symb == SEPARATOR || symb == '\n' || symb == '\0') state = STPARS_END;
            break;
        }
        case STPARS_QUOTE: {
            if(symb == '"') state = STPARS_SECQUOTE;
            break;
        }
        case STPARS_SECQUOTE: {   
            if(symb == '"') state = STPARS_QUOTE;
            else if(symb == SEPARATOR || symb == '\n' || symb == '\0') state = STPARS_END;
            else state = STPARS_ERROR;
            break;
        }
        case STPARS_END: {
            return index;
            break;
        }
        default:{ 
            return -1;
            break;
        }
        }
    }
    if(state == STPARS_ERROR || state == STPARS_QUOTE) return -1;
    return index + 1;
}
char unquote_cell(char *cell_start, long *len){
    StateParsing state = STPARS_START;
    long indexN = 0, index = 0;
    for(; cell_start[index]; index++, indexN++)
    {
        cell_start[indexN] = cell_start[index];
        char symb = cell_start[index];
        switch (state)
        {
        case STPARS_START: {
            if(symb == '"') {state = STPARS_QUOTE; indexN--;} 
            else if(symb == SEPARATOR || symb == '\n' || symb == '\r' || symb == '\0') state = STPARS_END;
            else {state = STPARS_NOQUOTE; }
            break;
        }
        case STPARS_NOQUOTE: {    
            if(symb == '"') state = STPARS_ERROR;
            else if(symb == SEPARATOR || symb == '\n' || symb == '\r' || symb == '\0') state = STPARS_END;
            break;
        }
        case STPARS_QUOTE: {
            if(symb == '"') {state = STPARS_SECQUOTE; indexN--;}
            break;
        }
        case STPARS_SECQUOTE: {   
            if(symb == '"') state = STPARS_QUOTE;
            else if(symb == SEPARATOR || symb == '\n' || symb == '\r' || symb == '\0') state = STPARS_END;
            else state = STPARS_ERROR;
            break;
        }
        case STPARS_END: {
            
            if (cell_start[index - 1] == '\r' && cell_start[index] == '\n')
                index++;
            char ret = cell_start[index - 1];
            cell_start[indexN - 1] = '\0';
            *len = index;
            return ret;
        }
        default:{ 
            return -1;
        }
        }
        
    }
    if(state == STPARS_ERROR || state == STPARS_QUOTE) return -1;
    *len = index + 1;
    return cell_start[index];
}


// пока берёт за эталон 1 строку и лишнее отсекает а недостоющее заполняет NULL 
int load_csv_file(const char *filepath, Table *out_table) {
    FILE *f;
    f = fopen(filepath, "rb"); 
    if(f == NULL) return -1;   
    
    if(fseek(f, 0, SEEK_END)) return -1;
    long file_size = ftell(f);
    if(fseek(f, 0, SEEK_SET)) return -1;

    char *buffer = malloc(file_size + 1);
    if (buffer) {
        fread(buffer, 1, file_size, f);
    }
    while(file_size > 1 && (buffer[file_size - 1] == '\n' || buffer[file_size - 1] == '\r')) 
        file_size--;
    buffer[file_size] = '\0';
    fclose(f);

    out_table->text_buffer = buffer;
    
    int rows = 1, cols = 1;
    long index = 0;
    while (index < file_size) {
        long len = len_cell(&buffer[index]);
        if(len == -1){
            table_free(out_table);
            return -2;
        }
        
        index += len;
        if(rows == 1 && buffer[index - 1] == SEPARATOR) {
            cols++;
        }
        else if(buffer[index - 1] == '\n') {
            rows++;
        }
    }

    out_table->cols = cols;
    out_table->rows = rows;
    out_table->cells = calloc(rows * cols, sizeof(Cell));

    index = 0;
    rows = 0, cols = 0;
    char skip = 0;
    while (rows < out_table->rows && index < file_size) {
        long len;
        char last = unquote_cell(&buffer[index], &len);
        
        if(last == -1) {
            table_free(out_table);
            return -2;
        }
        
        if(!skip) {
            if(cols < out_table->cols)
                out_table->cells[rows * out_table->cols + cols].raw = &buffer[index];
            else
                skip = 1;
            
            cols++;
        }
        if(last == '\n' || last == '\r') {
            skip = 0;
            cols = 0;
            rows++;
        }
        index += len;
    }

    return 0;
}

void print_sell(Table *table, int r, int c) {
    Cell cell = table->cells[r * table->cols + c];
    if(cell.raw != NULL) {
        if(cell.raw[0] == '=') {
            int value;
            int err = evaluate_cell(table, r, c, &value);
            if(err == -1) {
                printf("#CYCL!"); 
            }
            else if(err == -2) {
                printf("%s",cell.raw);
            }
            else if(err == -3) {
                printf("#DIV/0!"); 
            }
            else {
                printf("%d",value);
            }
        }
        else {
            printf("%s",cell.raw);
        }
    }
}

void print_csv_table(Table *table){
    for(int i = 0; i < table->rows; i++) {
        for(int j = 0; j < table->cols - 1; j++){
            print_sell(table, i, j);
            printf(",");
        }
        print_sell(table, i, table->cols - 1);
        printf("\n");
    }
}
