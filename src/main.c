#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "http_response.h"
#include "http_request.h"


int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};

	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
  if (client_fd == -1) {
    printf("Accept failed: %s\n", strerror(errno));
    return 1;
  }
	printf("Client connected\n");
  
  while (true) {
    // Listen for request from the client
    char buffer[1024] = {0};
    size_t bytes_read = 0;
    size_t total_bytes_read = 0;
    
    while ((bytes_read = read(client_fd, buffer + total_bytes_read, sizeof(buffer) - total_bytes_read - 1)) > 0) {
        total_bytes_read += bytes_read;
        buffer[total_bytes_read] = '\0';
        
        // Check if we already read the CLRF
        if (strstr(buffer, "\r\n\r\n")) {
            break;
        }
        
        if (total_bytes_read + 1 >= sizeof(buffer)) {
            break;
        }
    }
    
    struct http_request request;
    int success = parse_request(&request, buffer, total_bytes_read);
    if (success < 0) {
        perror("Failed parsing the request");
        continue;  // Skip to next iteration on error
    }
    
    // Default status line
    char* status_line = "HTTP/1.1 404 Not Found";
    
    // Navigate through each header of the object and extract the path
    if (request.request_line != NULL) {
        // The request line has format "METHOD PATH HTTP/VERSION"
        char* req_line_copy = strdup(request.request_line);
        char* method_str = strtok(req_line_copy, " ");
        char* path = strtok(NULL, " ");
        
        printf("Path requested: %s\n", path);
        
        // Check path and set appropriate response
        if (path != NULL && strcmp(path, "/") == 0) {
            // Root path - return 200 OK
            status_line = "HTTP/1.1 200 OK";
        }
        
        free(req_line_copy);  // Free the duplicated request line
    }
    
    // Navigate through each header
    for(int i = 0; i < request.header_count; i++) {
        char* header_copy = strdup(request.headers[i]);
        
        // Extract the name and value
        char* header_name = strtok(header_copy, ":");
        char* header_value = strtok(NULL, "");
        
        if (header_name && header_value) {
            // Trim the header value
            while (*header_value == ' ') {
                header_value++;
            }
            printf("Header: %s = %s\n", header_name, header_value);
        }
        
        free(header_copy);
    }
    
    // Create a response object
    struct http_response response = {
        .status_line = status_line,
    };
    
    char response_str[1024] = {0};
    size_t response_size = parse_response(&response, response_str, sizeof(response_str));  
    
    // Send the response to the client
    ssize_t bytes_sent = write(client_fd, response_str, response_size);
    if (bytes_sent < 0) {
        perror("Failed to send response");
    }
    
  }
  
	close(server_fd);

	return 0;
}
