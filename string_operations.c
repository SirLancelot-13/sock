#include <stdio.h>
#include <string.h>

char *send_get_request(char *host, char *endpoint, char *content){
    char *req=malloc(1024);
    sprintf(req, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Type: application/xml\r\nContent-length: %d\r\n%s\r\n\r\n",endpoint,host,strlen(content),content);
    return *req;
}

char *encode_response_text(char *response_text){

}