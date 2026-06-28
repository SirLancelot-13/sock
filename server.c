#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "server.h"
#include <sqlite3.h>
#include <arpa/inet.h>


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
        perror("Fucked up binding string\n");
    }
    if (sqlite3_bind_text(stmt, 2, ip_addr , -1, SQLITE_STATIC) != SQLITE_OK){
        perror("Fucked up binding string\n");
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

struct Server server_constructor(int domain, int port, int service, int protocol, int backlog, unsigned long interface, void (*launch)(struct Server *server))
{
    struct Server server;
    server.domain = domain;
    server.port = port;
    server.service = service;
    server.protocol = protocol;
    server.backlog = backlog;
    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.sock_fd = socket(domain, service, protocol);

    if (server.sock_fd < 0)
    {
        perror("Initialization got shitted on\n");
        exit(1);
    }

    if (bind(server.sock_fd, (struct sockaddr *)&server.address, sizeof(server.address)) < 0)
    {
        perror("Binding to socket failed gng/n");
        exit(1);
    }

    if (listen(server.sock_fd, server.backlog) < 0)
    {
        perror("Failed to start listening \n");
        exit(1);
    }

    server.launch = launch;
    return server;
}

void launch(struct Server *server)
{
    char buffer[BUFFER_SIZE];
    while (1)
    {   
        char str[INET_ADDRSTRLEN];
        printf(" Waiting for Connection...");
        int addrlen = sizeof(server->address);
        struct sockaddr_in client_address;
        int new_socket = accept(server->sock_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen);
        ssize_t bytesRead = read(new_socket, buffer, BUFFER_SIZE - 1);
        inet_ntop(AF_INET, &(client_address.sin_addr.s_addr), str, INET_ADDRSTRLEN);

        printf("IP Address of the client: %s\n\n",str);



        if (bytesRead >= 0)
        {
            buffer[bytesRead] = '\0';
            puts(buffer);
        }
        else
        {
            perror("Couldn't read buffer.");
        }

        //Using the new_socket here as it is non-blocking.
        char *response = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                         "<!DOCTYPE html>\r\n"
                         "<html>\r\n"
                         "<head>\r\n"
                         "<title>Testing Basic HTTP-SERVER</title>\r\n"
                         "</head>\r\n"
                         "<body>\r\n"
                         "Hello, World!\r\n"
                         "</body>\r\n"
                         "</html>\r\n";
        write(new_socket, response, strlen(response));
        close(new_socket); 
    }
}