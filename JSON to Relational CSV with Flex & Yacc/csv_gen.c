#include "csv_gen.h"
#include <sys/stat.h>
#include <errno.h>

/* Create a CSV context for data extraction and generation */
CsvContext* create_csv_context(SchemaContext *schema) {
    CsvContext *context = (CsvContext*)malloc(sizeof(CsvContext));
    if (!context) {
        fprintf(stderr, "Memory allocation failed for CSV context\n");
        exit(1);
    }
    
    context->schema = schema;
    context->next_id = 1;
    context->tables = NULL;  /* Initialize tables list */
    
    return context;
}

/* Find or create a table data structure for a schema */
TableData* find_or_create_table_data(CsvContext *context, Table *schema) {
    /* First, look for an existing table data */
    TableData *table_data = context->tables;
    while (table_data) {
        if (table_data->schema == schema) {
            return table_data;
        }
        table_data = table_data->next;
    }
    
    /* Create a new table data structure */
    table_data = (TableData*)malloc(sizeof(TableData));
    if (!table_data) {
        fprintf(stderr, "Memory allocation failed for table data\n");
        exit(1);
    }
    
    table_data->schema = schema;
    table_data->rows = NULL;
    table_data->next = NULL;
    
    /* Add to the context's table data list */
    if (context->tables == NULL) {
        context->tables = table_data;
    } else {
        TableData *current = context->tables;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = table_data;
    }
    
    return table_data;
}

/* Create a row data structure */
RowData* create_row_data(JsonValue *data, int id, int parent_id, int array_index) {
    RowData *row = (RowData*)malloc(sizeof(RowData));
    if (!row) {
        fprintf(stderr, "Memory allocation failed for row data\n");
        exit(1);
    }
    
    row->id = id;
    row->parent_id = parent_id;
    row->array_index = array_index;
    row->data = data;
    row->next = NULL;
    
    return row;
}

/* Add a row to a table */
void add_row_to_table(TableData *table_data, RowData *row) {
    if (table_data->rows == NULL) {
        table_data->rows = row;
    } else {
        RowData *current = table_data->rows;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = row;
    }
}

/* Process object data and extract rows */
void process_object_data(CsvContext *context, JsonValue *object, TableData *parent_data, int parent_id, int array_index) {
    /* Mark parent_data as unused to avoid warning */
    (void)parent_data;
    
    if (object->type != JSON_OBJECT) {
        return;
    }
    
    /* Generate a signature for this object */
    char *signature = generate_object_signature(object);
    
    /* Find the table schema for this object */
    Table *table_schema = find_table_by_signature(context->schema, signature);
    if (!table_schema) {
        fprintf(stderr, "Error: Table schema not found for object\n");
        free(signature);
        exit(1);
    }
    
    /* Find or create the table data */
    TableData *table_data = find_or_create_table_data(context, table_schema);
    
    /* Create a row for this object */
    int id = context->next_id++;
    RowData *row = create_row_data(object, id, parent_id, array_index);
    add_row_to_table(table_data, row);
    
    /* Process all key-value pairs */
    KeyValuePair *pair = object->value.object_head;
    while (pair) {
        switch (pair->value->type) {
            case JSON_OBJECT:
                /* Recursive call for nested objects */
                process_object_data(context, pair->value, table_data, id, -1);
                break;
                
            case JSON_ARRAY:
                /* Process arrays */
                process_array_data(context, pair->value, table_data, id, pair->key);
                break;
                
            default:
                /* Scalar values are handled later when writing the CSV */
                break;
        }
        
        pair = pair->next;
    }
    
    free(signature);
}

/* Process array data and extract rows */
void process_array_data(CsvContext *context, JsonValue *array, TableData *parent_data, int parent_id, const char *array_key) {
    if (array->type != JSON_ARRAY) {
        return;
    }
    
    /* Check the first element to determine if it's an array of objects or scalars */
    ArrayElement *elem = array->value.array_head;
    if (!elem) {
        return;  /* Empty array */
    }
    
    if (elem->value->type == JSON_OBJECT) {
        /* Array of objects - create rows in the child table */
        int index = 0;
        while (elem) {
            process_object_data(context, elem->value, parent_data, parent_id, index++);
            elem = elem->next;
        }
    } else {
        /* Array of scalars - create rows in the junction table */
        char signature[256];
        sprintf(signature, "junction:%s", array_key);
        
        /* Find the junction table schema */
        Table *junction_schema = find_table_by_signature(context->schema, signature);
        if (!junction_schema) {
            fprintf(stderr, "Error: Junction table schema not found for array\n");
            exit(1);
        }
        
        /* Find or create the junction table data */
        TableData *junction_data = find_or_create_table_data(context, junction_schema);
        
        /* Add a row for each scalar in the array */
        int index = 0;
        while (elem) {
            /* Create a row with a reference to the scalar value */
            int id = context->next_id++;
            RowData *row = create_row_data(elem->value, id, parent_id, index++);
            add_row_to_table(junction_data, row);
            
            elem = elem->next;
        }
    }
}

/* Extract data from the AST into the CSV context */
void extract_data(CsvContext *context, JsonValue *root) {
    /* Process the root object */
    process_object_data(context, root, NULL, 0, -1);
}

/* Escape a CSV field (handle quotes, commas, etc.) */
char* escape_csv_field(const char *field) {
    if (!field) {
        return strdup("");
    }
    
    /* Count characters that need escaping */
    int len = strlen(field);
    int needs_quotes = 0;
    int quotes_count = 0;
    
    for (int i = 0; i < len; i++) {
        if (field[i] == '"') {
            quotes_count++;
            needs_quotes = 1;
        } else if (field[i] == ',' || field[i] == '\n' || field[i] == '\r') {
            needs_quotes = 1;
        }
    }
    
    if (!needs_quotes && quotes_count == 0) {
        return strdup(field);
    }
    
    /* Allocate memory for the escaped field */
    char *escaped = (char*)malloc(len + quotes_count + 3);  /* +2 for surrounding quotes, +1 for null terminator */
    if (!escaped) {
        fprintf(stderr, "Memory allocation failed for escaped CSV field\n");
        exit(1);
    }
    
    /* Build the escaped field */
    int pos = 0;
    escaped[pos++] = '"';
    
    for (int i = 0; i < len; i++) {
        if (field[i] == '"') {
            escaped[pos++] = '"';  /* Double the quote */
            escaped[pos++] = '"';
        } else {
            escaped[pos++] = field[i];
        }
    }
    
    escaped[pos++] = '"';
    escaped[pos] = '\0';
    
    return escaped;
}

/* Write a single CSV file for a table */
void write_csv_file(TableData *table_data, const char *output_dir) {
    if (!table_data || !table_data->schema) {
        return;
    }
    
    /* Create the output filename */
    char filename[512];
    sprintf(filename, "%s/%s.csv", output_dir, table_data->schema->name);
    
    /* Open the file for writing */
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error opening file %s for writing: %s\n", filename, strerror(errno));
        exit(1);
    }
    
    /* Write the header row */
    Column *col = table_data->schema->columns;
    int first_col = 1;
    
    while (col) {
        if (!first_col) {
            fprintf(file, ",");
        }
        fprintf(file, "%s", col->name);
        first_col = 0;
        col = col->next;
    }
    fprintf(file, "\n");
    
    /* Write each data row */
    RowData *row = table_data->rows;
    while (row) {
        /* Reset for each row */
        col = table_data->schema->columns;
        first_col = 1;
        
        while (col) {
            if (!first_col) {
                fprintf(file, ",");
            }
            
            /* Output based on column type */
            switch (col->type) {
                case COL_ID:
                    fprintf(file, "%d", row->id);
                    break;
                    
                case COL_FOREIGN_KEY:
                    if (strcmp(col->name, "seq") == 0) {
                        fprintf(file, "%d", row->array_index);
                    } else {
                        fprintf(file, "%d", row->parent_id);
                    }
                    break;
                    
                case COL_INDEX:
                    fprintf(file, "%d", row->array_index);
                    break;
                    
                case COL_STRING:
                case COL_NUMBER:
                case COL_BOOLEAN:
                case COL_NULL:
                    /* Find the value for this column in the row data */
                    if (row->data->type == JSON_OBJECT) {
                        KeyValuePair *pair = row->data->value.object_head;
                        int found = 0;
                        
                        while (pair) {
                            if (strcmp(pair->key, col->name) == 0) {
                                found = 1;
                                
                                /* Output based on value type */
                                switch (pair->value->type) {
                                    case JSON_STRING: {
                                        char *escaped = escape_csv_field(pair->value->value.string_value);
                                        fprintf(file, "%s", escaped);
                                        free(escaped);
                                        break;
                                    }
                                    
                                    case JSON_NUMBER:
                                        fprintf(file, "%g", pair->value->value.number_value);
                                        break;
                                        
                                    case JSON_BOOLEAN:
                                        fprintf(file, "%s", pair->value->value.boolean_value ? "true" : "false");
                                        break;
                                        
                                    case JSON_NULL:
                                        /* Empty field for null */
                                        break;
                                        
                                    default:
                                        /* Shouldn't happen for scalar columns */
                                        break;
                                }
                                
                                break;
                            }
                            
                            pair = pair->next;
                        }
                        
                        if (!found && col->name[0] != '_') {
                            /* Column not found in this row */
                            /* Empty field (but should not happen with proper schema) */
                        }
                    } else if (row->data->type != JSON_OBJECT && strcmp(col->name, "value") == 0) {
                        /* For junction tables, output the scalar value */
                        switch (row->data->type) {
                            case JSON_STRING: {
                                char *escaped = escape_csv_field(row->data->value.string_value);
                                fprintf(file, "%s", escaped);
                                free(escaped);
                                break;
                            }
                            
                            case JSON_NUMBER:
                                fprintf(file, "%g", row->data->value.number_value);
                                break;
                                
                            case JSON_BOOLEAN:
                                fprintf(file, "%s", row->data->value.boolean_value ? "true" : "false");
                                break;
                                
                            case JSON_NULL:
                                /* Empty field for null */
                                break;
                                
                            default:
                                /* Shouldn't happen for scalar values */
                                break;
                        }
                    }
                    break;
            }
            
            first_col = 0;
            col = col->next;
        }
        
        fprintf(file, "\n");
        row = row->next;
    }
    
    fclose(file);
}

/* Write all CSV files */
void write_csv_files(CsvContext *context) {
    /* Ensure the output directory exists */
    struct stat st = {0};
    if (stat(context->schema->output_dir, &st) == -1) {
        if (mkdir(context->schema->output_dir, 0755) == -1) {
            fprintf(stderr, "Error creating directory %s: %s\n", 
                    context->schema->output_dir, strerror(errno));
            exit(1);
        }
    }
    
    /* Write a file for each table */
    TableData *table_data = context->tables;
    while (table_data) {
        write_csv_file(table_data, context->schema->output_dir);
        table_data = table_data->next;
    }
}

/* Generate all CSV files from the JSON AST */
void generate_csv_files(SchemaContext *schema, JsonValue *root) {
    /* Create the CSV context */
    CsvContext *context = create_csv_context(schema);
    
    /* Extract data from the AST */
    extract_data(context, root);
    
    /* Write the CSV files */
    write_csv_files(context);
    
    /* Clean up */
    free_csv_context(context);
}

/* Free memory for a CSV context */
void free_csv_context(CsvContext *context) {
    if (!context) {
        return;
    }
    
    /* Free table data */
    TableData *table_data = context->tables;
    while (table_data) {
        TableData *next_table = table_data->next;
        
        /* Free row data */
        RowData *row = table_data->rows;
        while (row) {
            RowData *next_row = row->next;
            free(row);
            row = next_row;
        }
        
        free(table_data);
        table_data = next_table;
    }
    
    free(context);
}
