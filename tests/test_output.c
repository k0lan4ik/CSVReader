#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    assert(f);
    fputs(content, f);
    fclose(f);
}

static char* read_all(const char* path) {
    FILE* f = fopen(path, "r");
    assert(f);
    
    size_t capacity = 1024;
    char* s = (char*)malloc(capacity);
    size_t len = 0;
    
    int c;
    while ((c = fgetc(f)) != EOF) {
        s[len++] = (char)c;
        if (len == capacity) {
            capacity *= 2;
            s = (char*)realloc(s, capacity);
        }
    }
    s[len] = '\0';
    fclose(f);
    return s;
}

static void run_output_test(const char* test_name, const char* input_csv, const char* expected_output) {
    printf("Running %s ", test_name);

    const char* in = "tmp_input.csv";
    const char* out = "tmp_output.txt";

    write_file(in, input_csv);

#ifdef _WIN32
    const char* cmd = "build\\bin\\csvreader.exe tmp_input.csv > tmp_output.txt";
#else
    const char* cmd = "./build/bin/csvreader tmp_input.csv > tmp_output.txt";
#endif

    int rc = system(cmd);
    assert(rc == 0);

    char* got = read_all(out);
    
    if (strcmp(got, expected_output) != 0) {
        printf("FAILED!\n");
        printf("--- EXPECTED ---\n%s", expected_output);
        printf("--- GOT ---\n%s", got);
        assert(0);
    }

    free(got);
    remove(in);
    remove(out);

    printf("OK\n");
}

int main(void) {
    printf("\nRunning Output Tests\n");

    //Тест1: пример
    run_output_test("test_example",
        ",A,B,Cell\n1,1,0,1\n2,2,=A1+Cell30,0\n30,0,=B1+A1,5\n",
        ",A,B,Cell\n1,1,0,1\n2,2,6,0\n30,0,1,5\n"
    );

    //Тест2:ошибки вычислений
    run_output_test("test_errors_print",
        ",A,B\n1,=10/0,=B1\n",
        ",A,B\n1,#DIV/0!,#CYCL!\n"
    );

    //Тест3: отрицательные числа и формулы
    run_output_test("test_negative_and_chains",
        ",A,B\n1,-5,10\n2,=A1*-2,=A2+B1\n",
        ",A,B\n1,-5,10\n2,10,20\n"
    );

    //тест4: одиночная ссылка 
    run_output_test("test_single_reference",
        ",A\n1,42\n2,=A1\n",
        ",A\n1,42\n2,42\n"
    );

    //Тест5: несуществующая ссылка или неверный синтаксис
    run_output_test("test_invalid_syntax",
        ",A\n1,=Z99\n2,=10+^5\n",
        ",A\n1,=Z99\n2,=10+^5\n"
    );

    printf("All output tests passed!\n");
    return 0;
}