#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct JsonNode {
    char *key;                 // For object properties; NULL for array elements
    JsonType type;             // Type of value
    union {
        double number;
        char *string;
        int boolean;
    } value;
    struct JsonNode **children; // Array of child nodes (for objects/arrays)
    int child_count;            // Number of children
} JsonNode;

void printHello(void);

#endif