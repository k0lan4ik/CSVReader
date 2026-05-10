#ifndef CSVREADER_H
#define CSVREADER_H
#include "csvtable.h"
#include "mathparse.h"

#define SEPARATOR ','


int load_csv_file(const char *filepath, Table *out_table);
void print_csv_table(Table *table);

typedef enum {
    STPARS_START = 0,
    STPARS_NOQUOTE,
    STPARS_QUOTE,
    STPARS_SECQUOTE,
    STPARS_END,
    STPARS_ERROR = -1,
}StateParsing;

#endif