#include <jist.h>
#include <stdio.h>

const char *parse(jist_output *output, const void *buffer, size_t bytes, const char *charset) {
    (void)buffer;
    char *size_str = NULL;
    asprintf(&size_str, "%zu", bytes);

    jist_output_insert(output, "Format", "txt");
    jist_output_insert(output, "Size", size_str);
    jist_output_insert(output, "Encoding", charset);

    return NULL;
}
