#ifndef JIST_H
#define JIST_H

#include <stddef.h>
#include <stdint.h>

// The jist output structure
typedef struct {
    char *key;
    char *value;
} jist_entry;
typedef struct {
    jist_entry **entries;
    size_t length;
    size_t capacity;
} jist_output;

jist_output jist_output_new();
void jist_output_insert(jist_output *self, const char *key, const char *value);
void jist_output_free(jist_output *self);

// Create custom error strings with jist_errorf()
const char *jist_errorf(const char *format, ...);

#endif
