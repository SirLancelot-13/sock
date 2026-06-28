#include <stdio.h>
#include <string.h>

char *send_post_request(char *host, char *endpoint, char *content){
    char *req=malloc(1024);
    snprintf(req, "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Type: application/xml\r\nContent-Length: %d\r\n%s\r\n\r\n",endpoint,host,strlen(content),content);
    return req;
}

char *encode_response_text(char *response_text){

}