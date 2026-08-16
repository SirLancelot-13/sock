#include "db_operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


sqlite3 *initialize_db() {
    sqlite3 *db = NULL;
    const char *db_path = getenv("DB_PATH");
    if (!db_path) {
        /* Default to workspace root database.db when running from server/ */
        db_path = "../database.db";
    }

    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        perror("database failed to open\n");
        exit(1);
    }

    char *err_msg = NULL;
    const char *create_username_sql =
        "CREATE TABLE IF NOT EXISTS username("
        "usname TEXT, "
        "ip_addr TEXT, "
        "CONSTRAINT ip_addr_unique UNIQUE(ip_addr)"
        ");";
    if (sqlite3_exec(db, create_username_sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to create username table: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    const char *create_messages_sql =
        "CREATE TABLE IF NOT EXISTS messages("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT, "
        "message TEXT"
        ");";
    if (sqlite3_exec(db, create_messages_sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to create messages table: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    return db;
}

int insert_or_ignore(sqlite3 *db, const char* ip_addr, const char* usname){
    sqlite3_stmt *stmt;
    const char *sql = "insert or ignore into username (usname, ip_addr) values (?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)!=SQLITE_OK){
        perror("Failed to prepare the sqlite statement.\n");
    }

    if (sqlite3_bind_text(stmt, 1, usname , -1, SQLITE_STATIC) != SQLITE_OK){
        perror("Messed up binding string\n");
    }
    if (sqlite3_bind_text(stmt, 2, ip_addr , -1, SQLITE_STATIC) != SQLITE_OK){
        perror("Messed up binding string\n");
    }

    int rc=sqlite3_step(stmt);
    if (rc==SQLITE_DONE){
        if (sqlite3_changes64(db)>0){
            printf("Row successfully added to the table.\n");
        }
        else {
            printf("IP already exists in the database.\n");
        }
    } else {
        printf("Execution error: %s\n", sqlite3_errmsg(db));
        perror("Failed execution");
    }

    sqlite3_finalize(stmt);
    return (rc==SQLITE_DONE);
}

char *get_username(sqlite3 *db, const char* ip_addr){
    sqlite3_stmt *stmt;
    const char *sql = "select usname from username where ip_addr=?";
    char *username = NULL;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        perror("Couldn't prepare query\n");
        return NULL;
    }

    if (sqlite3_bind_text(stmt, 1, ip_addr, -1, SQLITE_STATIC) != SQLITE_OK) {
        perror("couldn't bind text to IP query\n");
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text) {
            username = strdup((const char *)text);
        }
    }

    sqlite3_finalize(stmt);
    return username;
}

int insert_message(sqlite3 *db, const char *username, const char *message) {
    sqlite3_stmt *stmt;
    const char *sql = "insert into messages (username, message) values (?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        perror("Failed to prepare the insert message statement.\n");
        return 0;
    }

    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        perror("Failed to bind username\n");
    }
    if (sqlite3_bind_text(stmt, 2, message, -1, SQLITE_STATIC) != SQLITE_OK) {
        perror("Failed to bind message\n");
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        printf("Execution error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

char *get_last_10_messages(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT username, message FROM (SELECT id, username, message FROM messages ORDER BY id DESC LIMIT 10) ORDER BY id ASC;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        perror("Failed to prepare get_last_10_messages statement.\n");
        return NULL;
    }

    size_t capacity = 1024;
    char *result = malloc(capacity);
    if (!result) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    result[0] = '\0';
    size_t length = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *uname = sqlite3_column_text(stmt, 0);
        const unsigned char *msg = sqlite3_column_text(stmt, 1);

        const char *display_uname = uname ? (const char *)uname : "anonymous";
        const char *display_msg = msg ? (const char *)msg : "";

        size_t needed = strlen(display_uname) + strlen(display_msg) + 5;
        if (length + needed >= capacity) {
            capacity *= 2;
            char *new_result = realloc(result, capacity);
            if (!new_result) {
                free(result);
                sqlite3_finalize(stmt);
                return NULL;
            }
            result = new_result;
        }

        int written = snprintf(result + length, capacity - length, "[%s]: %s\n", display_uname, display_msg);
        if (written > 0) {
            length += written;
        }
    }

    sqlite3_finalize(stmt);
    return result;
}
