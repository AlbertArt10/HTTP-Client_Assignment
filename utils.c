#include "utils.h"

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

buffer buffer_init(void)
{
    buffer buffer;

    buffer.data = NULL;
    buffer.size = 0;

    return buffer;
}

void buffer_destroy(buffer *buffer)
{
    if (buffer->data != NULL) {
        free(buffer->data);
        buffer->data = NULL;
    }

    buffer->size = 0;
}

int buffer_is_empty(buffer *buffer)
{
    return buffer->data == NULL;
}

void buffer_add(buffer *buffer, const char *data, size_t data_size)
{
    if (buffer->data != NULL) {
        buffer->data = realloc(buffer->data, (buffer->size + data_size) * sizeof(char));
    } else {
        buffer->data = calloc(data_size, sizeof(char));
    }

    memcpy(buffer->data + buffer->size, data, data_size);

    buffer->size += data_size;
}

int buffer_find(buffer *buffer, const char *data, size_t data_size)
{
    if (data_size > buffer->size)
        return -1;

    size_t last_pos = buffer->size - data_size + 1;

    for (size_t i = 0; i < last_pos; ++i) {
        size_t j;

        for (j = 0; j < data_size; ++j) {
            if (buffer->data[i + j] != data[j]) {
                break;
            }
        }

        if (j == data_size)
            return i;
    }

    return -1;
}

int buffer_find_insensitive(buffer *buffer, const char *data, size_t data_size)
{
    if (data_size > buffer->size)
        return -1;

    size_t last_pos = buffer->size - data_size + 1;

    for (size_t i = 0; i < last_pos; ++i) {
        size_t j;

        for (j = 0; j < data_size; ++j) {
            if (tolower(buffer->data[i + j]) != tolower(data[j])) {
                break;
            }
        }

        if (j == data_size)
            return i;
    }

    return -1;
}

int is_convertable_to_num(char *str) {
    for (int i = 0; i < strlen(str); i++)
        if (str[i] < '0' || str[i] > '9')
            return 0;
    return 1;
}

void register_user() {
    char *msg = calloc(LINELEN, sizeof(char));

    // Read input
    char username[20], password[20];
    printf("username=");
    fgets(username, 20, stdin);
    printf("password=");
    fgets(password, 20, stdin);

    // Create the json payload for register
    JSON_Value *json_val = json_value_init_object();
    JSON_Object *json_obj = json_value_get_object(json_val);
    json_object_set_string(json_obj, "username", username);
    json_object_set_string(json_obj, "password", password);
    // Serialize
    char *payload = json_serialize_to_string(json_val);
    json_value_free(json_val);

    // Compute the POST request
    char** body_data = &payload;
    compute_post_string(SERVER_ADDRESS, "/api/v1/tema/auth/register", "application/json", body_data, 1, NULL, 0, NULL, msg);

    // Open connection
    // Send to server and await response
    int sock_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("Socket error\n"); 
        return;
    }
    send_to_server(sock_fd, msg);
    char *reply = receive_from_server(sock_fd);
    close_connection(sock_fd);

    if (strstr(reply, "OK") != NULL)
        printf("SUCCESS - Register successful.\n");
    else 
        printf("ERROR - Username is used.\n");

    free(reply);
    free(payload);
    free(msg);
}

void login_user(char **cookie_auth)
{
    char *msg = calloc(LINELEN, sizeof(char));

    // Read input
    char username[30], password[30];
    printf("username=");
    fgets(username, 30, stdin);
    printf("password=");
    fgets(password, 30, stdin);

    // Create the json payload for register
    JSON_Value *json_val = json_value_init_object();
    JSON_Object *json_obj = json_value_get_object(json_val);
    json_object_set_string(json_obj, "username", username);
    json_object_set_string(json_obj, "password", password);

    char *payload = json_serialize_to_string(json_val);
    json_value_free(json_val);

    // Compute the POST request
    compute_post_string(SERVER_ADDRESS, "/api/v1/tema/auth/login", "application/json", &payload, 1, NULL, 0, NULL, msg);

    // Open connection
    // Send to server and await response
    int sock_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sock_fd, msg);
    char *reply = receive_from_server(sock_fd);
    close_connection(sock_fd);

    if (strstr(reply, "OK") != NULL)
        printf("SUCCESS - Login successful.\n");
    else {
        printf("ERROR - Incorrect credentials.\n");
        return; // No cookie to extract
    }

    // Get the cookie from the reply
    if (*cookie_auth != NULL) {
        // Notify user that their previous account has logged out
        printf("WARNING - You have logged out of your previous account.\n");
    }
    free(*cookie_auth); // Free previous cookie

    // Get cookie
    char *cookie = strstr(reply, "Set-Cookie: ");
    cookie += strlen("Set-Cookie: ");
    char *end = strchr(cookie, ';');
    *cookie_auth = malloc(500);
    strncpy(*cookie_auth, cookie, end - cookie);
    (*cookie_auth)[end - cookie] = 0;

    free(reply);
    free(payload);
    free(msg);
}

void enter_library(char *cookie_auth, char **token) {
    if (cookie_auth == NULL) {
        printf("ERROR - You are not logged in.\n");
        return;
    }

    char *msg = calloc(LINELEN, sizeof(char));
    compute_get_string(SERVER_ADDRESS, "/api/v1/tema/library/access", NULL, &cookie_auth, 1, NULL, msg);

    // Open connection
    // Send to server and await response
    int sock_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sock_fd, msg);
    char *reply = receive_from_server(sock_fd);
    close_connection(sock_fd);

    if (strstr(reply, "OK") == NULL) {
        printf("ERROR - You are not logged in.\n");
        return; // No token to extract
    }

    printf("SUCCESS - Gained access to library.\n");

    // Get the token
    free(*token); // free previous token
    char *token_str = strstr(reply, "token\":\"");
    token_str += strlen("token\":\"");
    char *end = strchr(reply, '}');
    *token = malloc(500);
    strncpy(*token, token_str, end - token_str - 1);
    (*token)[end - token_str - 1] = 0;

    free(reply);
    free(msg);
}

void get_books(char *token) {
    if (token == NULL) {
        printf("ERROR - You have not gained access to library.\n");
        return;
    }

    char *msg = calloc(LINELEN, sizeof(char));

    // Compute GET request
    compute_get_string(SERVER_ADDRESS, "/api/v1/tema/library/books", NULL, NULL, 0, token, msg);

    int sock_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sock_fd, msg);
    char *reply = receive_from_server(sock_fd);
    close_connection(sock_fd);

    if (strstr(reply, "OK") == NULL) {
        printf("ERROR - Access Denied. Please execute get_library.\n");
        return;
    }

    // Get books from reply
    char *begin = strchr(reply, '[');
    char *end = strchr(begin, ']');
    char *books = malloc(500);
    strncpy(books, begin, end - begin + 1);
    books[end - begin + 1] = 0;

    // Print the books in a pretty format
    JSON_Value *root_value = json_parse_string(books);
    printf("%s\n", json_serialize_to_string_pretty(root_value));

    free(books);
    free(msg);
    free(reply);
}

void add_book(char *token) {
    if (token == NULL) {
        printf("ERROR - You are not logged in.\n");
        return;
    }

    // Read book attributes
    char *msg = calloc(LINELEN, sizeof(char));
    char title[100], author[100], genre[100], publisher[100], page_count_input[5];
    printf("title=");
    fgets(title, 100, stdin); 
    printf("author=");
    fgets(author, 100, stdin); 
    printf("genre=");
    fgets(genre, 100, stdin); 
    printf("publisher=");   
    fgets(publisher, 100, stdin);
    printf("page_count=");  
    fgets(page_count_input, 5, stdin);

    // For every input remove the endline character
    if (title[strlen(title) - 1] == '\n')
        title[strlen(title) - 1] = 0;
    if (author[strlen(author) - 1] == '\n')
        author[strlen(author) - 1] = 0;
    if (genre[strlen(genre) - 1] == '\n')
        genre[strlen(genre) - 1] = 0;
    if (publisher[strlen(publisher) - 1] == '\n')
        publisher[strlen(publisher) - 1] = 0;
    if (page_count_input[strlen(page_count_input) - 1] == '\n')
        page_count_input[strlen(page_count_input) - 1] = 0;

    // check if either field is empty
    if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0 || strlen(page_count_input) == 0) {
        printf("ERROR - All fields must be filled.\n");
        return;
    }

    // Check if page_count is a number
    if (!is_convertable_to_num(page_count_input)) {
        printf("ERROR - Enter a number for the page count.\n");
        return;
    }

    int page_count_num = atoi(page_count_input);
    // Create the json payload for adding a book
    JSON_Value *json_val = json_value_init_object();
    JSON_Object *json_obj = json_value_get_object(json_val);
    json_object_set_string(json_obj, "title", title);
    json_object_set_string(json_obj, "author", author);
    json_object_set_string(json_obj, "genre", genre);
    json_object_set_number(json_obj, "page_count", (double)page_count_num);
    json_object_set_string(json_obj, "publisher", publisher);
    char *payload = json_serialize_to_string(json_val);
    json_value_free(json_val);

    // Compute POST request
    compute_post_string(SERVER_ADDRESS, "/api/v1/tema/library/books", "application/json", &payload, 1, NULL, 0, token, msg);

    int sock_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sock_fd, msg);
    char *reply = receive_from_server(sock_fd);
    close_connection(sock_fd);

    if (strstr(reply, "OK") == NULL) {
        printf("ERROR - Access Denied. Please execute enter_library.\n");
        return;
    }
    printf("SUCCESS - Book has been added to the library.\n");

    free(reply);
    free(payload);
    free(msg);
}

void get_book(char *token) {
    if (token == NULL) {
        printf("ERROR - You are not logged in.\n");
        return;
    }

    char *msg = calloc(LINELEN, sizeof(char));

    // Read input
    char id_str[20];
    printf("id=");
    fgets(id_str, 20, stdin); 
    id_str[strlen(id_str) - 1] = 0;
    
    if (!is_convertable_to_num(id_str)) {
        printf("ERROR - enter a number.\n");
        return;
    }

    char url_complete[100] = "/api/v1/tema/library/books/";
    strcat(url_complete, id_str);
    compute_get_string(SERVER_ADDRESS, url_complete, NULL, NULL, 0, token, msg);

    int sock_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sock_fd, msg);
    char *reply = receive_from_server(sock_fd);
    close_connection(sock_fd);

    // Check token validity
    if (strstr(reply, "OK") == NULL) {
        printf("ERROR - Access Denied. Please execute enter_library.\n");
        return;
    }
    
    // Extract book from reply
    char *begin = strstr(reply, "{");
    char *end = strstr(begin, "}");
    char *book = malloc(500);
    strncpy(book, begin, end - begin + 1);
    book[end - begin + 1] = 0;

    if (strstr(book, "No book was found") != NULL) {
        printf("Could not find book with id: %d.\n", atoi(id_str));
        return;
    }

    // Print book in a pretty format
    JSON_Value *root_value = json_parse_string(book);
    printf("%s\n", json_serialize_to_string_pretty(root_value));

    free(reply);
    free(msg);
    free(book);
}

void delete_book(char *token) {
    if (token == NULL) {
        printf("ERROR - You are not logged in.\n");
        return;
    }

    char *msg = calloc(LINELEN, sizeof(char));

    char id[50];
    printf("id=");
    fgets(id, 50, stdin); 
    id[strlen(id) - 1] = 0;

    if (!is_convertable_to_num(id)) {
        printf("ERROR - Enter a valid number for the id.\n");
        return;
    }

    char url_complete[100] = "/api/v1/tema/library/books/";
    strcat(url_complete, id);
    compute_delete_string(SERVER_ADDRESS, url_complete, NULL, NULL, 0, token, msg);

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, msg);
    char *reply = receive_from_server(sockfd);
    close_connection(sockfd);

    // Check token validity
    if (strstr(reply, "OK") == NULL) {
        printf("ERROR - Access Denied. Please execute enter_library.\n");
        return;
    }

    if (strstr(reply, "No book was deleted"))
        printf("ERROR - Could not find book with id: %d\n", atoi(id));
    else
        printf("SUCCESS - Book deleted successfully.\n");

    free(reply);
    free(msg);
}

void logout(char **cookie_auth) {
    if (*cookie_auth == NULL) {
        printf("ERROR - You are not logged in.\n");
        return;
    }

    char *msg = calloc(LINELEN, sizeof(char));
    compute_get_string(SERVER_ADDRESS, "/api/v1/tema/auth/logout", NULL, cookie_auth, 1, NULL, msg);

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, msg);
    char *reply = receive_from_server(sockfd);
    close_connection(sockfd);

    if (strstr(reply, "OK") != NULL)
        printf("SUCCESS - You have logged out.\n");
    else
        printf("ERROR - You are not logged in!\n");

    free(*cookie_auth); // Free cookie. clear session
    *cookie_auth = NULL;
    free(reply);
    free(msg);
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_msg(char *msg, const char *line)
{
    strcat(msg, line);
    strcat(msg, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *msg)
{
    int bytes, sent = 0;
    int total = strlen(msg);

    do
    {
        bytes = write(sockfd, msg + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing msg to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0){
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
        
        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;
            
            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
            
            if (content_length_start < 0) {
                continue;           
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t) header_end;
    
    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0) {
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}

void compute_get_string(char *host, char *url, char *query_params, char **cookies, 
                            int cookies_count, char *token, char *msg)
{
    int i;
    char *line = malloc(LINELEN);
    memset(line, 0, LINELEN);

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_msg(msg, line);

    sprintf(line, "Host: %s", host);
    compute_msg(msg, line);

    sprintf(line, "User-Agent: Mozilla/5.0");
    compute_msg(msg, line);

    sprintf(line, "Connection: keep-alive");
    compute_msg(msg, line);

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_msg(msg, line);
    }

    if (cookies != NULL) {
        sprintf(line, "Cookie: ");

        for(i = 0; i < cookies_count - 1; i++)
            strcat(line, cookies[i]);

        strcat(line, cookies[cookies_count - 1]);
        compute_msg(msg, line);
    }

    compute_msg(msg, "");
    free(line);
}

void compute_post_string(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char *token, char *msg)
{
    int i, len;
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_msg(msg, line);
    
    sprintf(line, "Host: %s", host);
    compute_msg(msg, line);

    sprintf(line, "Connection: keep-alive");
    compute_msg(msg, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_msg(msg, line);

    len = 0;
    for(i = 0; i < body_data_fields_count - 1; i++){
        len += strlen(body_data[i]) + 1;
    }
    len += strlen(body_data[body_data_fields_count - 1]);

    sprintf(line, "Content-Length: %d", len);
    compute_msg(msg, line);
    
    if (cookies != NULL) {
       sprintf(line, "Cookie: ");

        for(i = 0; i < cookies_count - 1; i++)
            strcat(line, cookies[i]);

        strcat(line, cookies[cookies_count - 1]);
        compute_msg(msg, line);
    }

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_msg(msg, line);
    }

    compute_msg(msg, "");

    for(i = 0; i < body_data_fields_count - 1; i++)
        strcat(body_data_buffer, body_data[i]);

    strcat(body_data_buffer, body_data[body_data_fields_count - 1]);

    compute_msg(msg, body_data_buffer);

    free(line);
    free(body_data_buffer);
}

void compute_delete_string(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token, char *msg)
{
    // Reset buffer
    memset(msg, 0, LINELEN);

    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL)
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    else
        sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_msg(msg, line);

    sprintf(line, "Host: %s", host);
    compute_msg(msg, line);

    sprintf(line, "User-Agent: Mozilla/5.0");
    compute_msg(msg, line);

    sprintf(line, "Connection: keep-alive");
    compute_msg(msg, line);

    if (cookies != NULL) {
        strcpy(line, "Cookie: ");
        
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_msg(msg, line);
    }

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_msg(msg, line);
    }

    compute_msg(msg, "");

    free(line);
}
