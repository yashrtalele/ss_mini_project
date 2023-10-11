#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#define SERVER_PORT 8888
#define MAX_CONNECTIONS 100

enum user_type { ADMIN, STUDENT, FACULTY };

typedef struct {
    int id;
    enum user_type type;
    char username[128];
    char password[128];
} user_t;

typedef struct {
    int socket_fd;
    user_t user;
} client_connection_t;

typedef struct {
    int id;
    char name[128];
    int credits;
    char instructor[128];
} course_t;

pthread_mutex_t user_db_mutex;

// Define the user database
user_t user_db[100];

// Define the course database
course_t course_db[100];

// Load the user database from a file
void load_user_db() {
    // Open the user database file
    FILE *fp = fopen("/home/yash/ss_mini_project/db/users.db", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    // Read each user from the file and add it to the database
    while (!feof(fp)) {
        user_t user;
        fscanf(fp, "%d %d %s %s\n", &user.id, &user.type, user.username, user.password);
        user_db[user.id] = user;
    }
    // Close the user database file
    fclose(fp);
}

// Save the user database to a file
void save_user_db() {
    // Open the user database file
    FILE *fp = fopen("/home/yash/ss_mini_project/db/users.db", "w");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    // Write each user from the database to the file
    for (int i = 0; i < 100; i++) {
        user_t user = user_db[i];
        if (user.username[0] != '\0') {
            fprintf(fp, "%d %d %s %s\n", user.id, user.type, user.username, user.password);
        }
    }
    // Close the user database file
    fclose(fp);
}

// Authenticate a user
int authenticate_user(char *username, char *password) {
    // Lock the user database mutex
    pthread_mutex_lock(&user_db_mutex);
    // Search for the user in the database
    int i, id;
    for (i = 0; i < 100; i++) {
        user_t user = user_db[i];
        if (strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
            id=user.id;
            break;
        }
    }
    // Unlock the user database mutex
    pthread_mutex_unlock(&user_db_mutex);
    // If the user was found, return their ID, otherwise return -1
    return (i < 100) ? id : -1;
}

// Create a new user
int create_user(char *username, char *password, enum user_type type) {
     // Lock the user database mutex
    pthread_mutex_lock(&user_db_mutex);
    // Find the next available user ID
    int id = 0;
    while (user_db[id].username[0] != '\0') {
        id++;
    }
    // Create a new user
    user_t user = { id, type, *username, *password };
    user_db[id] = user;
    // Unlock the user database mutex
    pthread_mutex_unlock(&user_db_mutex);
    // Return the new user ID
    return id;
}

void *handle_client_connection(void *args) {
    // * Get the client connection information
    client_connection_t *client_connection = (client_connection_t *) args;
    // ...

    char choice[1];
    if((recv(client_connection->socket_fd, choice, sizeof(choice), 0)) < 0) {
        perror("recv");
        // retry
    }
    int ch=atoi(choice);
    client_connection->user.type=ch-1;
    // char uname[100]={0};
    if((recv(client_connection->socket_fd, client_connection->user.username, sizeof(client_connection->user.username), 0)) < 0) {
        perror("recv");
        // retry
    }
    if((recv(client_connection->socket_fd, client_connection->user.password, sizeof(client_connection->user.password), 0)) < 0) {
        perror("recv");
        // retry
    }
    // ? authenticate
    int user_id=authenticate_user(client_connection->user.username, client_connection->user.password);
    if (user_id < 0) {
        if((send(client_connection->socket_fd, "Failed to authenticate user. Please create your user account!\n", 63, 0)) < 0) {
            perror("send");
            close(client_connection->socket_fd);
            exit(EXIT_FAILURE);
        }
        close(client_connection->socket_fd);
        // free(client_connection);
        // return NULL;
    }

    // TODO: process user's requests;

    close(client_connection->socket_fd);
    // free(client_connection);
    // return NULL;
}

void handle_client_con(void *args) {
    // * Get the client connection information
    client_connection_t *client_connection = (client_connection_t *) args;
    // ...
    // ? authenticate
    int user_id=authenticate_user(client_connection->user.username, client_connection->user.password);
    if (user_id <= 0) {
        if((send(client_connection->socket_fd, "Failed to authenticate user. Please create your user account!\n", 63, 0)) < 0) {
            perror("send");
            close(client_connection->socket_fd);
            exit(EXIT_FAILURE);
        }
        create_user(client_connection->user.username, client_connection->user.password, client_connection->user.type);
        save_user_db();
        close(client_connection->socket_fd);
        // free(client_connection);
    }

    // TODO: process user's requests;

    close(client_connection->socket_fd);
    free(client_connection);
}

void main(void) {
    // SOCKET CONNECTION -----
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(SERVER_PORT);
    server_address.sin_addr.s_addr=INADDR_ANY;

    int bind_result=bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if(bind_result < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    listen(server_socket, MAX_CONNECTIONS);
    load_user_db();
    while (1) {
        // Accept a new connection
        client_connection_t *client_connection = malloc(sizeof(client_connection_t));
        client_connection->socket_fd = accept(server_socket, NULL, NULL);
        if (client_connection->socket_fd < 0) {
            perror("accept");
            exit(1);
        }
        int send_ack=send(client_connection->socket_fd, "Connection Success!", 20, 0);
        if(send_ack < 0) {
            perror("send");
            // retry
        }
        
        // handle_client_con(client_connection);
        // Create a new thread to handle the connection
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client_connection, client_connection);
    }

    // Close the server socket
    close(server_socket);
}