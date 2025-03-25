#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the response of an http
struct http_request {
  char* request_line; // eg: HTTP/1.1 200 OK
  char** headers; // Array of string for each headers
  size_t header_count;
  char* body;
  size_t body_length;
};

int parse_request(struct http_request* request, char* buffer, size_t buffer_size);

#endif
