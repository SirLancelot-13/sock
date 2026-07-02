#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>


sqlite3 *db;
void initialize_db(){
    if ( sqlite3_open("database.db", &db) != 0){
        perror("database failed to open idk\n");
        exit(1);
    };
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
    const char *sql="select usname from username where ip_addr='?'";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)!=SQLITE_OK){
        perror("Couldn't prepare query\n");
    }
    if (sqlite3_bind_text(stmt, 1, ip_addr, -1, SQLITE_STATIC)){
        perror("couldn't bind text to IP query\n");
    }
    int rc=sqlite3_step(stmt);
    if (rc==SQLITE_ROW){
        return sqlite3_column_text(stmt, 0);
    }
    else {
        return "username_not_found";
    }
}
