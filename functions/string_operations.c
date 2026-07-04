#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_operations.h"

enum Request get_request_type(char *request) {
  if (strncmp(request, "GET", 3) == 0) {
    return get;
  } else if (strncmp(request, "POST", 4) == 0) {
    return post;
  }
  return unknown;
}


char *send_post_request(char *host, char *endpoint, char *content) {
  char *req = malloc(1024);
  snprintf(
      req, 1024,
      "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Type: "
      "application/xml\r\nContent-Length: %zu\r\n%s\r\n\r\n",
      endpoint, host, strlen(content), content);
  return req;
}

char *send_get_request(char *host, char *endpoint) {
  char *req = malloc(1024);
  snprintf(req, 1024,
           "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n",
           endpoint, host);
  return req;
}

char *encode_get_response_text(char *content) {
  char *res = malloc(1024);
  snprintf(res, 1024,
           "HTTP/1.1 200 OK\r\nCache-Control: no-cache\r\nConnection: "
           "keep-alive\r\nContent-Type: application/xml\r\nContent-Length: "
           "%zu\r\n%s",
           strlen(content), content);
  return res;
}

char *get_host_name(char *request, enum Request r_type) {
  char *content;
  char *host;
  char *endpoint;
  if (get_request_type(request) != r_type) {
    return NULL;
  }
  if (r_type == get) {
    sscanf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n%s", endpoint, host,
           content);
  } else if (r_type == post) {
    sscanf(request, "POST %s HTTP/1.1\r\nHost: %s\r\n%s", endpoint, host,
           content);
  } else {
    return NULL;
  }
  return host;
}

char *extract_endpoint_type_shi(char *request) {
  char *endpoint = malloc(128);
  enum Request r_type = get_request_type(request);
  if (r_type == get) {
    sscanf(request, "GET %127s", endpoint);
  } else if (r_type == post) {
    sscanf(request, "POST %127s", endpoint);
  } else {
    free(endpoint);
    return NULL;
  }
  return endpoint;
}