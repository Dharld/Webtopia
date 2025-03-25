#include "http_request.h"

int parse_request(struct http_request* request, char* buffer, size_t buffer_size) {
    if (!request || !buffer) return -1;
    
    // Initialize the request structure
    request->request_line = NULL;
    request->headers = NULL;
    request->header_count = 0;
    request->body = NULL;
    
    char* buffer_copy = strdup(buffer);
    if (!buffer_copy) return -1;
    
    // Read the request line
    char* request_line = strtok(buffer_copy, "\r\n");
    if (request_line != NULL) {
        request->request_line = strdup(request_line);
        
        char* header_line;
        while ((header_line = strtok(NULL, "\r\n")) != NULL) {
            if (header_line[0] == '\0') {
                break;
            }
            // Read all the headers
            request->header_count += 1;
            request->headers = realloc(request->headers, request->header_count * sizeof(char*));
            if (request->headers == NULL) {
                perror("Failed to allocate memory");
                break;
            } else {
                request->headers[request->header_count - 1] = strdup(header_line);
            }
        }
        
        // Read the body
        char* double_clrf = strstr(buffer, "\r\n\r\n");
        if (double_clrf != NULL) {
            request->body = strdup(double_clrf + 4); // Skip the four delimiters
        }
    }
    
    // Free the temporary buffer copy
    free(buffer_copy);
    
    return 0; // Success
}
