#ifndef SCHEMA_H
#define SCHEMA_H

#include "ast.h"

/* Column types for our schema */
typedef enum {
    COL_ID,           /* Primary key */
    COL_FOREIGN_KEY,  /* Foreign key to parent table */
    COL_INDEX,        /* Array index */
    COL_STRING,       /* String value */
    COL_NUMBER,       /* Number value */
    COL_BOOLEAN,      /* Boolean value */
    COL_NULL          /* Null value */
} ColumnType;

/* Column definition */
typedef struct Column {
    char *name;
    ColumnType type;
    struct Column *next;
} Column;

/* Table definition */
typedef struct Table {
    char *name;
    Column *columns;
    struct Table *next;
    char *parent_table;  /* Name of parent table, if any */
    char *object_signature;  /* Signature of object shape */
} Table;

/* Schema context */
typedef struct SchemaContext {
    Table *tables;
    char *output_dir;
    int print_ast;
} SchemaContext;

/* Schema detection functions */
SchemaContext* create_schema_context(const char *output_dir, int print_ast);
void detect_schema(SchemaContext *context, JsonValue *root);
void free_schema_context(SchemaContext *context);

/* Helper functions */
char* generate_object_signature(JsonValue *object);
Table* find_or_create_table(SchemaContext *context, const char *name, const char *object_signature);
void add_column(Table *table, const char *name, ColumnType type);
Table* find_table_by_signature(SchemaContext *context, const char *signature);

#endif /* SCHEMA_H */
