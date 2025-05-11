#ifndef CSV_GEN_H
#define CSV_GEN_H

#include "schema.h"

/* Forward declarations */
typedef struct TableData TableData;
typedef struct RowData RowData;

/* CSV generation context */
typedef struct CsvContext {
    SchemaContext *schema;
    int next_id;  /* For generating sequential IDs */
    TableData *tables;  /* List of table data */
} CsvContext;

/* Row data for CSV output */
struct RowData {
    int id;
    int parent_id;
    int array_index;
    JsonValue *data;
    RowData *next;
};

/* Table data for CSV output */
struct TableData {
    Table *schema;
    RowData *rows;
    TableData *next;
};

/* CSV generation functions */
void generate_csv_files(SchemaContext *schema, JsonValue *root);
CsvContext* create_csv_context(SchemaContext *schema);
void extract_data(CsvContext *context, JsonValue *root);
void write_csv_files(CsvContext *context);
void free_csv_context(CsvContext *context);

/* Helper functions */
void process_object_data(CsvContext *context, JsonValue *object, TableData *parent_data, int parent_id, int array_index);
void process_array_data(CsvContext *context, JsonValue *array, TableData *parent_data, int parent_id, const char *array_key);
TableData* find_or_create_table_data(CsvContext *context, Table *schema);
RowData* create_row_data(JsonValue *data, int id, int parent_id, int array_index);
void add_row_to_table(TableData *table_data, RowData *row);
void write_csv_file(TableData *table_data, const char *output_dir);
char* escape_csv_field(const char *field);

#endif /* CSV_GEN_H */
