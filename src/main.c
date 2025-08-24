#include <asm-generic/errno-base.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <jist.h>
#include <linux/limits.h>
#include <log.h>
#include <magic.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

const char *app_name;
typedef const char *(*jist_parse)(jist_output *output, const void *buffer, size_t bytes, const char *charset);

void get_mime_and_charset(int fd, char **mime, char **charset) {
    magic_t cookie = magic_open(MAGIC_MIME);
    if (cookie == NULL) {
        log_fatal("Unable to initialize libmagic");
    }
    if (magic_load(cookie, NULL) != 0) {
        log_fatal("Unable to load libmagic database: %s", magic_error(cookie));
    }
    const char *magic = magic_descriptor(cookie, fd);
    if (magic == NULL) {
        log_fatal("Unable to get mimetype: %s", magic_error(cookie));
    }

    // Mime type looks like this
    // text/plain; charset=us-ascii
    //
    // We first get the text/plain
    // Then the us-ascii
    char *mime_string = strdup(magic);
    if (*mime = strtok(mime_string, "; "), mime == NULL) {
        log_fatal("Unable to parse mimetype (mime)");
    }
    if (*charset = strtok(NULL, "; "), charset == NULL) {
        log_fatal("Unable to parse mimetype (charset)");
    }
    if (*charset = strchr(*charset, '='), charset == NULL) {
        log_fatal("Unable to parse mimetype (charset)");
    }
    *mime = strdup(*mime);
    *charset = strdup(*charset + 1);

    free(mime_string);
    magic_close(cookie);
}
int main(int argc, const char *argv[]) {
    // TLDR:
    // Ensure the arguments are correct
    // Determine the paths
    // Open the database
    // Open the file
    // Find the mimetype and charset
    // Query the database
    // Find the plugin
    // Load the plugin (dlopen, dlsym, dlclose)
    // Parse the data
    // Print the data

    // Ensure the arguments are correct
    app_name = argv[0];
    if (argc < 2) {
        log_fatal("Insufficent number of arguments");
    }

    // Determine the paths
    char *config_path = NULL;
    char *data_path = NULL;
    asprintf(&config_path, "%s/.config/jist", getenv("HOME"));
    asprintf(&data_path, "%s/.local/share/jist", getenv("HOME"));
    if (mkdir(config_path, 0777) == -1 && errno != EEXIST) {
        free(config_path);
        free(data_path);
        log_fatal("Unable to make: %s", strerror(errno));
    }
    if (mkdir(data_path, 0777) == -1 && errno != EEXIST) {
        free(config_path);
        free(data_path);
        log_fatal("Unable to make: %s", strerror(errno));
    }

    // Open the database
    char *database_path = NULL;
    asprintf(&database_path, "%s/database.db", config_path);
    sqlite3 *db = NULL;
    if (sqlite3_open(database_path, &db) != SQLITE_OK) {
        free(config_path);
        free(data_path);
        free(database_path);
        log_fatal("Database error: %s", sqlite3_errmsg(db));
    }

    // Open the file
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        free(config_path);
        free(data_path);
        free(database_path);
        sqlite3_close(db);
        log_fatal("Unable to open the file: %s", strerror(errno));
    }

    // Find the mimetype and charset
    char *mime = NULL;
    char *charset = NULL;
    get_mime_and_charset(fd, &mime, &charset);

    // Query the database
    sqlite3_stmt *stmt = NULL;
    char *query = sqlite3_mprintf("SELECT * FROM plugins WHERE mime=%Q", mime);
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        free(config_path);
        free(data_path);
        free(database_path);
        free(mime);
        free(charset);
        sqlite3_free(query);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        close(fd);
        log_fatal("Unable to query database: %s", sqlite3_errmsg(db));
    }

    // Find the plugin
    const unsigned char *plugin = NULL;
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        // We create the table
        plugin = sqlite3_column_text(stmt, 1);
    }

    if (plugin == NULL) {
        free(config_path);
        free(data_path);
        free(database_path);
        free(mime);
        free(charset);
        sqlite3_free(query);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        close(fd);
        log_fatal("Cannot find the plugin");
    }

    // Load the plugin (dlopen, dlsym, dlclose)
    char *plugin_path = NULL;
    asprintf(&plugin_path, "%s/%s", data_path, plugin);
    void *handle = dlopen(plugin_path, RTLD_LAZY);
    if (handle == NULL) {
        free(config_path);
        free(data_path);
        free(database_path);
        free(plugin_path);
        free(mime);
        free(charset);
        sqlite3_free(query);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        close(fd);
        log_fatal("Unable to open the plugin: %s", dlerror());
    }

    jist_parse parse = dlsym(handle, "parse");
    if (parse == NULL) {
        free(config_path);
        free(data_path);
        free(database_path);
        free(plugin_path);
        free(mime);
        free(charset);
        sqlite3_free(query);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        close(fd);
        dlclose(handle);
        log_fatal("Unable to locate the 'parse' function in the plugin: %s", dlerror());
    }

    // Lets get the size
    struct stat s;
    if (fstat(fd, &s) == -1) {
        free(config_path);
        free(data_path);
        free(database_path);
        free(plugin_path);
        free(mime);
        free(charset);
        sqlite3_free(query);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        close(fd);
        dlclose(handle);
        log_fatal("Unable to stat the file: %s", strerror(errno));
    }
    // mmap the file
    void *buffer = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buffer == MAP_FAILED) {
        free(config_path);
        free(data_path);
        free(database_path);
        free(plugin_path);
        free(mime);
        free(charset);
        sqlite3_free(query);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        close(fd);
        dlclose(handle);
        munmap(buffer, s.st_size);
        log_fatal("Unable to mmap the file: %s", strerror(errno));
    }

    // Parse the data
    jist_output output = jist_output_new();
    const char *error = parse(&output, buffer, s.st_size, charset);
    if (error != NULL) {
        free(config_path);
        free(data_path);
        free(database_path);
        free(plugin_path);
        free(mime);
        free(charset);
        sqlite3_free(query);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        close(fd);
        dlclose(handle);
        munmap(buffer, s.st_size);
        jist_output_free(&output);
        log_fatal("Unable to parse file: %s", error);
    }

    // Print the output
    printf("%s:\n", argv[1]);
    for (size_t i = 0; i < output.length; i++) {
        jist_entry *entry = output.entries[i];
        printf("%s: %s\n", entry->key, entry->value);
    }

    // Cleanup
    free(config_path);
    free(data_path);
    free(database_path);
    free(plugin_path);
    free(mime);
    free(charset);
    sqlite3_free(query);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    close(fd);
    dlclose(handle);
    munmap(buffer, s.st_size);
    jist_output_free(&output);
}
