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




// пока берёт за эталон 1 строку и лишнее отсекает а недостоющее заполняет NULL 
int load_csv_file(const char *filepath, Table *out_table) {
   FILE *f;
   f = fopen(filepath, "rb"); 
   if(f == NULL) return -1;   
   
   if(fseek(f, 0, SEEK_END)) return -1;
   long fileSize = ftell(f);
   if(fseek(f, 0, SEEK_SET)) return -1;

   char *buffer = malloc(fileSize + 1);
   if (buffer) {
        fread(buffer, 1, fileSize, f);
   }
   buffer[fileSize] = '\0';
   fclose(f);

   out_table->text_buffer = buffer;
   
   int rows = 1, cols = 1;
   long index = 0;
   while (index < fileSize)
   {
       long len = len_cell(&buffer[index]);
       if(len == -1){
            table_free(out_table);
            return -2;
       }
       
       index += len;
       if(rows == 1 && buffer[index - 1] == SEPARATOR){
            cols++;
       }
       else if(buffer[index - 1] == '\n'){
            rows++;
       }
   }
   printf("%d | %d", cols, rows);
   

   return 0;
}

