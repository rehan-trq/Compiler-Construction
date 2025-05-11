#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* JSON value types */
typedef enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_NULL
} JsonType;

/* Forward declaration for JsonValue */
typedef struct JsonValue JsonValue;

/* Key-value pair for objects */
typedef struct KeyValuePair {
    char *key;
    JsonValue *value;
    struct KeyValuePair *next;  /* For linked list */
} KeyValuePair;

/* Array element */
typedef struct ArrayElement {
    JsonValue *value;
    struct ArrayElement *next;  /* For linked list */
} ArrayElement;

/* JSON value structure */
struct JsonValue {
    JsonType type;
    union {
        KeyValuePair *object_head;  /* For objects */
        ArrayElement *array_head;   /* For arrays */
        char *string_value;         /* For strings */
        double number_value;        /* For numbers */
        int boolean_value;          /* For booleans (0 or 1) */
    } value;
    int line;                       /* Line number for error reporting */
    int column;                     /* Column number for error reporting */
};

/* AST creation functions */
JsonValue* create_object(int line, int column);
JsonValue* create_array(int line, int column);
JsonValue* create_string(char *value, int line, int column);
JsonValue* create_number(double value, int line, int column);
JsonValue* create_boolean(int value, int line, int column);
JsonValue* create_null(int line, int column);

/* Object and array manipulation */
void add_key_value(JsonValue *object, char *key, JsonValue *value);
void add_array_element(JsonValue *array, JsonValue *element);

/* AST traversal and printing */
void print_ast(JsonValue *root, int indent);

/* Memory management */
void free_json_value(JsonValue *value);

#endif /* AST_H */
