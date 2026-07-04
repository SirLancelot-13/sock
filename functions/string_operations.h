#ifndef string_operations
#define string_operations

enum Request { get, post, unknown };

enum Request get_request_type(char *request);
char *send_post_request(char *host, char *endpoint, char *content);
char *send_get_request(char *host, char *endpoint);
char *encode_get_response_text(char *content);
char *get_host_name(char *request, enum Request r_type);
char *extract_endpoint_type_shi(char *request);
char *extract_post_content(const char *request);

#endif