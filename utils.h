#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "parson.h"

#define BUFLEN 4096
#define LINELEN 1000

#define SERVER_IP "34.246.184.49"
#define SERVER_PORT 8080
#define SERVER_ADDRESS "34.246.184.49:8080"

typedef struct {
    char *data;
    size_t size;
} buffer;

// initializes a buffer
buffer buffer_init(void);

// destroys a buffer
void buffer_destroy(buffer *buffer);

// adds data of size data_size to a buffer
void buffer_add(buffer *buffer, const char *data, size_t data_size);

// checks if a buffer is empty
int buffer_is_empty(buffer *buffer);

// finds data of size data_size in a buffer and returns its position
int buffer_find(buffer *buffer, const char *data, size_t data_size);

// case-insensitive fashion and returns its position
int buffer_find_insensitive(buffer *buffer, const char *data, size_t data_size);

void compute_get_string(char *host, char *url, char *query_params,
							char **cookies, int cookies_count, char *token, char *message);

void compute_post_string(char *host, char *url, char* content_type, char **body_data,
							int body_data_fields_count, char** cookies, int cookies_count, char *token, char* message);

void compute_delete_string(char *host, char *url, char *query_params, char **cookies, 
                        int cookies_count, char *token, char *message);

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

// register a user
void register_user();

// login a user
void login_user(char **cookie_auth);

// request access to library data (protected resources)
void enter_library(char *cookie_auth, char **token);

// get all books from the library
void get_books(char *token);

// get a book from the library via id
void get_book(char *token);

// add a book to the library
void add_book(char *token);

// delete a book from the library via id
void delete_book(char *token);

// logout a user
void logout(char **cookie_auth);

#endif