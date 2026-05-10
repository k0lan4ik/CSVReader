#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "csvtable.h"
#include "csvreader.h"
#include "mathparse.h"

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s ", #test_func); \
        test_func(); \
        printf("OK\n"); \
    } while(0)

void create_test_file(const char* filename, const char* content) {
    FILE* f = fopen(filename, "wb");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}


//Тест1: базоваяарифметика
void test_basic_math() {
    const char* filename = "test_math.csv";
    create_test_file(filename, ",A,B\n1,5,10\n2,=5+5,=B1*2\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);
    assert(table.rows == 3 && table.cols == 3);

    int val = 0;    

    err = evaluate_cell(&table, 1, 2, &val);
    assert(err == 0 && val == 10);

    err = evaluate_cell(&table, 2, 1, &val);
    assert(err == 0 && val == 10);
  
    err = evaluate_cell(&table, 2, 2, &val);
    assert(err == 0 && val == 20);

    table_free(&table);
    remove(filename);
}

//тест2: защита  от циклической зависимости
void test_circular_dependency() {
    const char* filename = "test_cycle.csv";
    create_test_file(filename, ",A,B\n1,=B1,=A1\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);

    int val = 0;
    err = evaluate_cell(&table, 1, 1, &val);
    assert(err == -1);

    table_free(&table);
    remove(filename);
}

//тест3: деление на ноль
void test_division_by_zero() {
    const char* filename = "test_div0.csv";
    create_test_file(filename, ",A\n1,0\n2,=10/A1\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);

    int val = 0;
    err = evaluate_cell(&table, 2, 1, &val);
    assert(err == -3);

    table_free(&table);
    remove(filename);
}
//Тест4: работа с кавычками 
void test_quotes_and_names() {
    const char* filename = "test_quotes.csv";
    create_test_file(filename, ",Col1,\"Col2\"\nRow1,1,=Col1Row1+5\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);

    int val = 0;
    err = evaluate_cell(&table, 1, 2, &val);
    assert(err == 0 && val == 6);

    table_free(&table);
    remove(filename);
}

//Тест5: отрицательные числа
void test_negative_numbers() {
    const char* filename = "test_neg.csv";
    create_test_file(filename, ",A\n1,-5\n2,=-10--5\n3,=A1*-2\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);

    int val = 0;
    err = evaluate_cell(&table, 1, 1, &val);
    assert(err == 0 && val == -5);

    err = evaluate_cell(&table, 2, 1, &val);
    assert(err == 0 && val == -5);

    err = evaluate_cell(&table, 3, 1, &val);
    assert(err == 0 && val == 10);

    table_free(&table);
    remove(filename);
}
//Тест6: несуществующая ячейка
void test_invalid_reference() {
    const char* filename = "test_inv_ref.csv";
    create_test_file(filename, ",A\n1,5\n2,=Z99+A1\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);

    int val = 0;
    err = evaluate_cell(&table, 2, 1, &val);
    assert(err == -2);

    table_free(&table);
    remove(filename);
}

//Тест7:неверный синтаксис формул
void test_invalid_formula_syntax() {
    const char* filename = "test_inv_form.csv";
    create_test_file(filename, ",A\n1,=5^2\n2,=5+\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);

    int val = 0;
    err = evaluate_cell(&table, 1, 1, &val);
    assert(err == -2);

    err = evaluate_cell(&table, 2, 1, &val);
    assert(err == -2);

    table_free(&table);
    remove(filename);
}

//Тест8 :запятые внутри кавычек
void test_commas_in_quotes() {
    const char* filename = "test_comma_quote.csv";
    create_test_file(filename, ",A,\"B,C\"\n1,\"10,0\",=A1+5\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);
    assert(table.cols == 3); 

    table_free(&table);
    remove(filename);
}

//Тест9: пустые ячейки
void test_empty_cells() {
    const char* filename = "test_empty.csv";
    create_test_file(filename, ",A,B\n1,,=A1+5\n");

    Table table = {0};
    int err = load_csv_file(filename, &table);
    assert(err == 0);

    int val = 0;
    err = evaluate_cell(&table, 1, 1, &val);
    assert(err == 0 && val == 0);

    err = evaluate_cell(&table, 1, 2, &val);
    assert(err == 0 && val == 5);

    table_free(&table);
    remove(filename);
}

//тест10:попытка открыть несуществующий файл
void test_file_not_found() {
    Table table = {0};
    int err = load_csv_file("this_file_does_not_exist_at_all.csv", &table);
    assert(err == -1); 
}

int main() {
    printf("\nRunning Math & CSV Tests\n");
    RUN_TEST(test_basic_math);
    RUN_TEST(test_circular_dependency);
    RUN_TEST(test_division_by_zero);
    RUN_TEST(test_quotes_and_names);
    RUN_TEST(test_negative_numbers);
    RUN_TEST(test_invalid_reference);
    RUN_TEST(test_invalid_formula_syntax);
    RUN_TEST(test_commas_in_quotes);
    RUN_TEST(test_empty_cells);
    RUN_TEST(test_file_not_found);
    printf("All tests passed!\n");
    return 0;
}