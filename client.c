#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "utils.h"

int main()
{
    // Security context
    char *cookie_auth = NULL;
    char *json_web_token = NULL;

    // Run until exit...
    while (1) {
        char command[30];
        fgets(command, 30, stdin);

        // Remove endline char
        if (command[strlen(command) - 1] == '\n')
            command[strlen(command) - 1] = 0;

        if (strcmp(command, "exit") == 0) {
            break;
        }
        if (strcmp(command, "register") == 0) {
            register_user();
            continue;
        }
        if (strcmp(command, "login") == 0) {
            login_user(&cookie_auth);
            continue;
        }
        if (strcmp(command, "enter_library") == 0) {
            enter_library(cookie_auth, &json_web_token);
            continue;
        }
        if (strcmp(command, "get_books") == 0) {
            get_books(json_web_token);
            continue;
        }
        if (strcmp(command, "get_book") == 0) {
            get_book(json_web_token);
            continue;
        }
        if (strcmp(command, "add_book") == 0) {
            add_book(json_web_token);
            continue;
        }
        if (strcmp(command, "delete_book") == 0) {
            delete_book(json_web_token);
            continue;
        }
        if (strcmp(command, "logout") == 0) {
            logout(&cookie_auth);
            continue;
        }
    }

    free(cookie_auth);
    free(json_web_token);
    return 0;
}