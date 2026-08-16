#ifndef db_operations_h
#define db_operations_h

#include <sqlite3.h>


sqlite3 *initialize_db();
int insert_or_ignore(sqlite3 *db, const char* ip_addr, const char* usname);
char *get_username(sqlite3 *db, const char* ip_addr);
int insert_message(sqlite3 *db, const char *username, const char *message);
char *get_last_10_messages(sqlite3 *db);

#endif
