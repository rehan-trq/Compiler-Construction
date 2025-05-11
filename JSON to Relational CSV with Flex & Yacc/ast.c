#include "ast.h"

/* Create a new JSON object node */
JsonValue* create_object(int line, int column) {
    JsonValue *obj = (JsonValue*)malloc(sizeof(JsonValue));
    if (!obj) {
        fprintf(stderr, "Memory allocation failed for JSON object\n");
        exit(1);
    }
    
    obj->type = JSON_OBJECT;
    obj->value.object_head = NULL;
    obj->line = line;
    obj->column = column;
    
    return obj;
}

/* Create a new JSON array node */
JsonValue* create_array(int line, int column) {
    JsonValue *arr = (JsonValue*)malloc(sizeof(JsonValue));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed for JSON array\n");
        exit(1);
    }
    
    arr->type = JSON_ARRAY;
    arr->value.array_head = NULL;
    arr->line = line;
    arr->column = column;
    
    return arr;
}

/* Create a new JSON string node */
JsonValue* create_string(char *value, int line, int column) {
    JsonValue *str = (JsonValue*)malloc(sizeof(JsonValue));
    if (!str) {
        fprintf(stderr, "Memory allocation failed for JSON string\n");
        exit(1);
    }
    
    str->type = JSON_STRING;
    str->value.string_value = strdup(value);
    if (!str->value.string_value) {
        fprintf(stderr, "Memory allocation failed for string value\n");
        free(str);
        exit(1);
    }
    str->line = line;
    str->column = column;
    
    return str;
}

/* Create a new JSON number node */
JsonValue* create_number(double value, int line, int column) {
    JsonValue *num = (JsonValue*)malloc(sizeof(JsonValue));
    if (!num) {
        fprintf(stderr, "Memory allocation failed for JSON number\n");
        exit(1);
    }
    
    num->type = JSON_NUMBER;
    num->value.number_value = value;
    num->line = line;
    num->column = column;
    
    return num;
}

/* Create a new JSON boolean node */
JsonValue* create_boolean(int value, int line, int column) {
    JsonValue *boolean = (JsonValue*)malloc(sizeof(JsonValue));
    if (!boolean) {
        fprintf(stderr, "Memory allocation failed for JSON boolean\n");
        exit(1);
    }
    
    boolean->type = JSON_BOOLEAN;
    boolean->value.boolean_value = value;
    boolean->line = line;
    boolean->column = column;
    
    return boolean;
}

/* Create a new JSON null node */
JsonValue* create_null(int line, int column) {
    JsonValue *null_val = (JsonValue*)malloc(sizeof(JsonValue));
    if (!null_val) {
        fprintf(stderr, "Memory allocation failed for JSON null\n");
        exit(1);
    }
    
    null_val->type = JSON_NULL;
    null_val->line = line;
    null_val->column = column;
    
    return null_val;
}

/* Add a key-value pair to a JSON object */
void add_key_value(JsonValue *object, char *key, JsonValue *value) {
    if (object->type != JSON_OBJECT) {
        fprintf(stderr, "Error: Cannot add key-value pair to non-object\n");
        exit(1);
    }
    
    KeyValuePair *pair = (KeyValuePair*)malloc(sizeof(KeyValuePair));
    if (!pair) {
        fprintf(stderr, "Memory allocation failed for key-value pair\n");
        exit(1);
    }
    
    pair->key = strdup(key);
    if (!pair->key) {
        fprintf(stderr, "Memory allocation failed for key\n");
        free(pair);
        exit(1);
    }
    
    pair->value = value;
    pair->next = NULL;
    
    /* Add to the end of the list to maintain order */
    if (object->value.object_head == NULL) {
        object->value.object_head = pair;
    } else {
        KeyValuePair *current = object->value.object_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = pair;
    }
}

/* Add an element to a JSON array */
void add_array_element(JsonValue *array, JsonValue *element) {
    if (array->type != JSON_ARRAY) {
        fprintf(stderr, "Error: Cannot add element to non-array\n");
        exit(1);
    }
    
    ArrayElement *arr_elem = (ArrayElement*)malloc(sizeof(ArrayElement));
    if (!arr_elem) {
        fprintf(stderr, "Memory allocation failed for array element\n");
        exit(1);
    }
    
    arr_elem->value = element;
    arr_elem->next = NULL;
    
    /* Add to the end of the list to maintain order */
    if (array->value.array_head == NULL) {
        array->value.array_head = arr_elem;
    } else {
        ArrayElement *current = array->value.array_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = arr_elem;
    }
}

/* Print the AST in an indented format */
void print_ast(JsonValue *root, int indent) {
    if (!root) {
        return;
    }
    
    char indent_str[256] = {0};
    for (int i = 0; i < indent; i++) {
        strcat(indent_str, "  ");
    }
    
    switch (root->type) {
        case JSON_OBJECT: {
            printf("%sOBJECT {\n", indent_str);
            KeyValuePair *pair = root->value.object_head;
            while (pair) {
                printf("%s  KEY: \"%s\"\n", indent_str, pair->key);
                printf("%s  VALUE: ", indent_str);
                print_ast(pair->value, indent + 2);
                pair = pair->next;
            }
            printf("%s}\n", indent_str);
            break;
        }
        
        case JSON_ARRAY: {
            printf("%sARRAY [\n", indent_str);
            ArrayElement *elem = root->value.array_head;
            int index = 0;
            while (elem) {
                printf("%s  [%d]: ", indent_str, index++);
                print_ast(elem->value, indent + 2);
                elem = elem->next;
            }
            printf("%s]\n", indent_str);
            break;
        }
        
        case JSON_STRING:
            printf("%sSTRING: \"%s\"\n", indent_str, root->value.string_value);
            break;
            
        case JSON_NUMBER:
            printf("%sNUMBER: %g\n", indent_str, root->value.number_value);
            break;
            
        case JSON_BOOLEAN:
            printf("%sBOOLEAN: %s\n", indent_str, root->value.boolean_value ? "true" : "false");
            break;
            
        case JSON_NULL:
            printf("%sNULL\n", indent_str);
            break;
            
        default:
            printf("%sUNKNOWN TYPE\n", indent_str);
            break;
    }
}

/* Free memory for a JSON value */
void free_json_value(JsonValue *value) {
    if (!value) {
        return;
    }
    
    switch (value->type) {
        case JSON_OBJECT: {
            KeyValuePair *pair = value->value.object_head;
            while (pair) {
                KeyValuePair *next = pair->next;
                free(pair->key);
                free_json_value(pair->value);
                free(pair);
                pair = next;
            }
            break;
        }
        
        case JSON_ARRAY: {
            ArrayElement *elem = value->value.array_head;
            while (elem) {
                ArrayElement *next = elem->next;
                free_json_value(elem->value);
                free(elem);
                elem = next;
            }
            break;
        }
        
        case JSON_STRING:
            free(value->value.string_value);
            break;
            
        default:
            /* Nothing to free for other types */
            break;
    }
    
    free(value);
}
