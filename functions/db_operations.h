#ifndef db_operations_h
#define db_operations_h

#include <sqlite3.h>

extern sqlite3 *db;

void initialize_db();
int insert_or_ignore(sqlite3 *db, const char* ip_addr, const char* usname);
char *get_username(sqlite3 *db, const char* ip_addr);

#endif
