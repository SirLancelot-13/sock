#include <stdio.h>
#include <string.h>

char *send_post_request(char *host, char *endpoint, char *content){
    char *req=malloc(1024);
    snprintf(req, 1024, "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Type: application/xml\r\nContent-Length: %d\r\n%s\r\n\r\n",endpoint,host,strlen(content),content);
    return req;
}

char *send_get_request(char *host, char *endpoint){
    char *req=malloc(1024);
    snprintf(req, 1024, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n",endpoint,host);
    return req;
}

char *encode_get_response_text(char *content){
    char *res=malloc(1024);
    snprintf(res, 1024, "HTTP/1.1 200 OK\r\nCache-Control: no-cache\r\nConnection: keep-alive\r\nContent-Type: application/xml\r\nContent-Length: %d\r\n%s",strlen(content),content);
    return res;
}