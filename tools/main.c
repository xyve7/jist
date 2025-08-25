#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <jist.h>
#include <libgen.h>
#include <linux/limits.h>
#include <log.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

const char *app_name;
int main(int argc, const char *argv[]) {
    app_name = argv[0];
    if (argc < 2) {
        log_fatal("Insufficent number of arguments");
    }

    if (strcmp(argv[1], "add") != 0) {
        log_fatal("Unknown command");
    }
    if (argc < 4) {

        log_fatal("(add) Insufficent number of arguments");
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

    // Copy the plugin
    int fd = open(argv[3], O_RDONLY);
    if (fd == -1) {
        free(config_path);
        free(data_path);
        free(database_path);
        sqlite3_close(db);
        log_fatal("Unable to open the file: %s", strerror(errno));
    }

    // Lets get the size
    struct stat s;
    if (fstat(fd, &s) == -1) {
        free(config_path);
        free(data_path);
        free(database_path);
        sqlite3_close(db);
        close(fd);
        log_fatal("Unable to stat the file: %s", strerror(errno));
    }
    // mmap the file
    void *buffer = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buffer == MAP_FAILED) {
        free(config_path);
        free(data_path);
        free(database_path);
        sqlite3_close(db);
        close(fd);
        log_fatal("Unable to mmap the file: %s", strerror(errno));
    }

    char *location = strdup(argv[3]);
    char *plugin = basename(location);
    char *plugin_path = NULL;
    asprintf(&plugin_path, "%s/%s", data_path, plugin);

    int to = open(plugin_path, O_WRONLY | O_CREAT);
    if (to == -1) {
        free(config_path);
        free(data_path);
        free(database_path);
        free(location);
        free(plugin_path);
        sqlite3_close(db);
        log_fatal("Unable to open the file: %s", strerror(errno));
    }
    write(to, buffer, s.st_size);
    close(to);

    // Query the database
    sqlite3_stmt *stmt = NULL;
    char *query = sqlite3_mprintf("INSERT INTO plugins VALUES(%Q, %Q)", argv[2], plugin);
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        free(config_path);
        free(data_path);
        free(database_path);
        sqlite3_free(query);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        log_fatal("Unable to query database: %s", sqlite3_errmsg(db));
    }

    free(config_path);
    free(data_path);
    free(database_path);
    free(plugin_path);
    free(location);
    sqlite3_free(query);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    close(fd);
    close(to);
    munmap(buffer, s.st_size);
}
