#include "schema.h"
#include <ctype.h>

/* Forward declarations */
void process_object(SchemaContext *context, JsonValue *object, Table *parent_table, const char *parent_key, int array_index);
void process_array(SchemaContext *context, JsonValue *array, Table *parent_table, const char *array_key);

/* Create a new schema context */
SchemaContext* create_schema_context(const char *output_dir, int print_ast) {
    SchemaContext *context = (SchemaContext*)malloc(sizeof(SchemaContext));
    if (!context) {
        fprintf(stderr, "Memory allocation failed for schema context\n");
        exit(1);
    }
    
    context->tables = NULL;
    context->print_ast = print_ast;
    
    if (output_dir) {
        context->output_dir = strdup(output_dir);
        if (!context->output_dir) {
            fprintf(stderr, "Memory allocation failed for output directory\n");
            free(context);
            exit(1);
        }
    } else {
        context->output_dir = strdup(".");  /* Default to current directory */
        if (!context->output_dir) {
            fprintf(stderr, "Memory allocation failed for output directory\n");
            free(context);
            exit(1);
        }
    }
    
    return context;
}

/* Generate a signature for an object shape (keys) */
char* generate_object_signature(JsonValue *object) {
    if (object->type != JSON_OBJECT) {
        return NULL;
    }
    
    /* Calculate required length for the signature */
    int sig_len = 1;  /* For null terminator */
    KeyValuePair *pair = object->value.object_head;
    while (pair) {
        sig_len += strlen(pair->key) + 1;  /* +1 for separator */
        pair = pair->next;
    }
    
    char *signature = (char*)malloc(sig_len);
    if (!signature) {
        fprintf(stderr, "Memory allocation failed for object signature\n");
        exit(1);
    }
    
    /* Build the signature as "key1,key2,key3" */
    signature[0] = '\0';
    pair = object->value.object_head;
    while (pair) {
        if (signature[0] != '\0') {
            strcat(signature, ",");
        }
        strcat(signature, pair->key);
        pair = pair->next;
    }
    
    return signature;
}

/* Find or create a table by name and signature */
Table* find_or_create_table(SchemaContext *context, const char *name, const char *object_signature) {
    /* First, look for an existing table with the same signature */
    Table *table = context->tables;
    while (table) {
        if (table->object_signature && strcmp(table->object_signature, object_signature) == 0) {
            return table;
        }
        table = table->next;
    }
    
    /* Create a new table */
    table = (Table*)malloc(sizeof(Table));
    if (!table) {
        fprintf(stderr, "Memory allocation failed for table\n");
        exit(1);
    }
    
    table->name = strdup(name);
    if (!table->name) {
        fprintf(stderr, "Memory allocation failed for table name\n");
        free(table);
        exit(1);
    }
    
    table->columns = NULL;
    table->next = NULL;
    table->parent_table = NULL;
    
    table->object_signature = strdup(object_signature);
    if (!table->object_signature) {
        fprintf(stderr, "Memory allocation failed for object signature\n");
        free(table->name);
        free(table);
        exit(1);
    }
    
    /* Add the ID column by default */
    add_column(table, "id", COL_ID);
    
    /* Add to the context's table list */
    if (context->tables == NULL) {
        context->tables = table;
    } else {
        Table *current = context->tables;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = table;
    }
    
    return table;
}

/* Add a column to a table */
void add_column(Table *table, const char *name, ColumnType type) {
    /* Check if the column already exists */
    Column *col = table->columns;
    while (col) {
        if (strcmp(col->name, name) == 0) {
            return;  /* Column already exists */
        }
        col = col->next;
    }
    
    /* Create a new column */
    Column *new_col = (Column*)malloc(sizeof(Column));
    if (!new_col) {
        fprintf(stderr, "Memory allocation failed for column\n");
        exit(1);
    }
    
    new_col->name = strdup(name);
    if (!new_col->name) {
        fprintf(stderr, "Memory allocation failed for column name\n");
        free(new_col);
        exit(1);
    }
    
    new_col->type = type;
    new_col->next = NULL;
    
    /* Add to the end of the column list */
    if (table->columns == NULL) {
        table->columns = new_col;
    } else {
        Column *current = table->columns;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_col;
    }
}

/* Find a table by its signature */
Table* find_table_by_signature(SchemaContext *context, const char *signature) {
    Table *table = context->tables;
    while (table) {
        if (table->object_signature && strcmp(table->object_signature, signature) == 0) {
            return table;
        }
        table = table->next;
    }
    return NULL;
}

/* Create a table name from an object key */
char* create_table_name(const char *key) {
    /* Convert key to lowercase and remove non-alphanumeric chars */
    char *name = strdup(key);
    if (!name) {
        fprintf(stderr, "Memory allocation failed for table name\n");
        exit(1);
    }
    
    /* Make it plural if it's not already */
    int len = strlen(name);
    if (len > 0 && name[len-1] != 's') {
        char *plural = (char*)malloc(len + 2);
        if (!plural) {
            fprintf(stderr, "Memory allocation failed for plural table name\n");
            free(name);
            exit(1);
        }
        strcpy(plural, name);
        strcat(plural, "s");
        free(name);
        return plural;
    }
    
    return name;
}

/* Process an object and add its fields to the schema */
void process_object(SchemaContext *context, JsonValue *object, Table *parent_table, const char *parent_key, int array_index) {
    if (object->type != JSON_OBJECT) {
        return;
    }
    
    /* Generate a signature for this object */
    char *signature = generate_object_signature(object);
    
    /* Determine table name */
    char *table_name;
    if (parent_table && parent_key) {
        table_name = create_table_name(parent_key);
    } else {
        /* Root object - use default name */
        table_name = strdup("root");
    }
    
    /* Find or create the table */
    Table *table = find_or_create_table(context, table_name, signature);
    free(table_name);
    
    /* If this is a nested object in an array, set the parent table */
    if (parent_table && array_index >= 0) {
        if (!table->parent_table) {
            table->parent_table = strdup(parent_table->name);
            
            /* Add foreign key column */
            char fk_name[256];
            sprintf(fk_name, "%s_id", parent_table->name);
            add_column(table, fk_name, COL_FOREIGN_KEY);
            
            /* Add index column if this is an array element */
            if (array_index >= 0) {
                add_column(table, "seq", COL_INDEX);
            }
        }
    }
    
    /* Process all key-value pairs */
    KeyValuePair *pair = object->value.object_head;
    while (pair) {
        switch (pair->value->type) {
            case JSON_OBJECT:
                /* Recursive call for nested objects */
                process_object(context, pair->value, table, pair->key, -1);
                
                /* Add a column for the foreign key to the nested object */
                char fk_name[256];
                sprintf(fk_name, "%s_id", pair->key);
                add_column(table, fk_name, COL_FOREIGN_KEY);
                break;
                
            case JSON_ARRAY:
                /* Process arrays */
                process_array(context, pair->value, table, pair->key);
                break;
                
            case JSON_STRING:
                add_column(table, pair->key, COL_STRING);
                break;
                
            case JSON_NUMBER:
                add_column(table, pair->key, COL_NUMBER);
                break;
                
            case JSON_BOOLEAN:
                add_column(table, pair->key, COL_BOOLEAN);
                break;
                
            case JSON_NULL:
                add_column(table, pair->key, COL_NULL);
                break;
        }
        
        pair = pair->next;
    }
    
    free(signature);
}

/* Process an array and create appropriate tables */
void process_array(SchemaContext *context, JsonValue *array, Table *parent_table, const char *array_key) {
    if (array->type != JSON_ARRAY) {
        return;
    }
    
    /* Check the first element to determine if it's an array of objects or scalars */
    ArrayElement *elem = array->value.array_head;
    if (!elem) {
        return;  /* Empty array */
    }
    
    if (elem->value->type == JSON_OBJECT) {
        /* Array of objects - create a child table */
        int index = 0;
        while (elem) {
            process_object(context, elem->value, parent_table, array_key, index++);
            elem = elem->next;
        }
    } else {
        /* Array of scalars - create a junction table */
        char table_name[256];
        sprintf(table_name, "%s", array_key);
        
        /* Create the junction table */
        char signature[256];
        sprintf(signature, "junction:%s", array_key);
        Table *junction = find_or_create_table(context, table_name, signature);
        
        /* Set parent table and add columns */
        if (!junction->parent_table) {
            junction->parent_table = strdup(parent_table->name);
            
            char fk_name[256];
            sprintf(fk_name, "%s_id", parent_table->name);
            add_column(junction, fk_name, COL_FOREIGN_KEY);
            add_column(junction, "index", COL_INDEX);
            add_column(junction, "value", COL_STRING);  /* Using string for all scalar values */
        }
    }
}

/* Detect schema from the AST */
void detect_schema(SchemaContext *context, JsonValue *root) {
    /* Print AST if requested */
    if (context->print_ast) {
        print_ast(root, 0);
    }
    
    /* Process the root object */
    process_object(context, root, NULL, NULL, -1);
}

/* Free memory for a schema context */
void free_schema_context(SchemaContext *context) {
    if (!context) {
        return;
    }
    
    /* Free tables */
    Table *table = context->tables;
    while (table) {
        Table *next_table = table->next;
        
        /* Free columns */
        Column *col = table->columns;
        while (col) {
            Column *next_col = col->next;
            free(col->name);
            free(col);
            col = next_col;
        }
        
        free(table->name);
        free(table->object_signature);
        if (table->parent_table) {
            free(table->parent_table);
        }
        free(table);
        
        table = next_table;
    }
    
    free(context->output_dir);
    free(context);
}
