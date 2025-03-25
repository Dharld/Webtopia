// Define the response of an http
struct http_response {
  char* status_line; // eg: HTTP/1.1 200 OK
  char** headers; // Array of string for each headers
  size_t header_count;
  char* body;
  size_t body_length;
} http_response;
