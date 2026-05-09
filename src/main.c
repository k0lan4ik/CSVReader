#include <stdio.h>
#include "csvtable.h"
#include "csvreader.h"
#include "mathparse.h"


int main(int argc, char *argv[]) {
    
    if(argc != 2)
    {
        printf("Error: Expected %d arguments, but 1 was provided.", argc);
        return 1;
    }

    Table table = {0};
    
    int err = load_csv_file(argv[1], &table);
    if (err == -1) {
        printf("Error: Cannot read file.\n");
        return 1;
    }
    if (err == -2) {
        printf("Error: Uncorrect file format.\n");
        return 1;
    }
    return 0;
}