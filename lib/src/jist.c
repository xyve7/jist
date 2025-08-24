#include <jist.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

jist_entry *new_entry(const char *key, const char *value) {
    jist_entry *self = malloc(sizeof(jist_entry));
    self->key = strdup(key);
    self->value = strdup(value);
    return self;
}
jist_output jist_output_new() {
    jist_output self;
    self.entries = malloc(10 * sizeof(jist_entry *));
    self.capacity = 10;
    self.length = 0;
    return self;
}
void jist_output_insert(jist_output *self, const char *key, const char *value) {
    if (self->length >= self->capacity) {
        self->capacity *= 2;
        self->entries = realloc(self->entries, self->capacity * sizeof(jist_entry *));
    }
    self->entries[self->length] = new_entry(key, value);
    self->length++;
}
void jist_output_free(jist_output *self) {
    for (size_t i = 0; i < self->length; i++) {
        jist_entry *entry = self->entries[i];
        free(entry->key);
        free(entry->value);
        free(entry);
    }
    free(self->entries);
    self->length = 0;
    self->capacity = 0;
    self->entries = NULL;
}

const char *jist_errorf(const char *format, ...) {
    char *error_string = NULL;
    va_list args;

    va_start(args, format);
    vasprintf(&error_string, format, args);
    va_end(args);

    return error_string;
}
