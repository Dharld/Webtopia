#include "http_response.h"

size_t parse_response(struct http_response* response, char* buffer, size_t buffer_size) {
  // Format the response in an appropriate body
  size_t size = 0;

  // Add the status line
  size += snprintf(buffer, buffer_size - size, "%s\r\n\r\n", response->status_line); 

  printf("%s\n", buffer);

  return size;
}
