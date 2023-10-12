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
    // FILE *fp = fopen("/home/yash/ss_mini_project/db/users.db", "r");
    // if (fp == NULL) {
    //     perror("fopen");
    //     exit(1);
    // }
    // // Read each user from the file and add it to the database
    // while (!feof(fp)) {
    //     user_t user;
    //     fscanf(fp, "%d %d %s %s\n", &user.id, &user.type, user.username, user.password);
    //     user_db[user.id] = user;
    // }
    // // Close the user database file
    // fclose(fp);
    int fd = open("/home/yash/ss_mini_project/db/users.db", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // Read each user from the file and add it to the database
    char buf[1024];
    while (1) {
        int bytes_read = read(fd, buf, sizeof(buf));
        if (bytes_read < 0) {
            perror("read");
            break;
        }

        if (bytes_read == 0) {
            break;
        }

        // Parse the user data from the buffer
        user_t user;
        sscanf(buf, "%d %d %s %s\n", &user.id, &user.type, user.username, user.password);
        user_db[user.id] = user;
    }
    close(fd);
}

// Save the user database to a file
void save_user_db() {
    // Open the user database file
    // FILE *fp = fopen("/home/yash/ss_mini_project/db/users.db", "w");
    // if (fp == NULL) {
    //     perror("fopen");
    //     exit(1);
    // }
    // printf("Inside save_user_db()\n");
    // // Write each user from the database to the file
    // for (int i = 0; i < 100; i++) {
    //     user_t user = user_db[i];
    //     if (user.username[0] != '\0') {
    //         printf("Inside for and if\n");
    //         fprintf(fp, "%d %d %s %s\n", user.id, user.type, user.username, user.password);
    //     }
    // }
    // // Close the user database file
    // fclose(fp);
    int fd = open("/home/yash/ss_mini_project/db/users.db", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    // printf("inside this");
    // Write each user from the database to the file
    char buf[1024];
    for (int i = 0; i < 100; i++) {
        user_t user = user_db[i];
        // printf("inside this for");
        if (user.username[0] != '\0') {
            sprintf(buf, "%d %d %s %s\n", user.id, user.type, user.username, user.password);
            int bytes_written = write(fd, buf, strlen(buf));
            if (bytes_written < 0) {
                perror("write");
                break;
            }
        }
    }

    // Close the user database file
    close(fd);
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
    user_t user;
    user.id=id;
    user.type=type;
    strncpy(user.username, username, strlen(username));
    strncpy(user.password, password, strlen(password));
    user_db[id] = user;
    printf("%s %s \n", user.username, user.password);
    // Unlock the user database mutex
    pthread_mutex_unlock(&user_db_mutex);
    // Return the new user ID
    printf("id = %d", id);
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
        if((send(client_connection->socket_fd, "Failed to authenticate user!\n", 30, 0)) < 0) {
            perror("send");
            // close(client_connection->socket_fd);
            exit(EXIT_FAILURE);
        }
        // create_user(client_connection->user.username, client_connection->user.password, client_connection->user.type);
        // save_user_db();
        close(client_connection->socket_fd);
        // free(client_connection);
        // return NULL;
    }
    send(client_connection->socket_fd, "Login Success!", 15, 0);
    // TODO: process user's requests;

    // close(client_connection->socket_fd);
    // free(client_connection);
    // return NULL;
}

void load_course_db() {
    // Open the course database file
    int course_db_fd = open("courses.txt", O_RDONLY);
    if (course_db_fd < 0) {
    perror("open");
    exit(1);
    }

    // Lock the course database file
    // TODO: File locking
    // fcntl(course_db_fd, F_SETLK, &course_db_lock);

    // Read each course from the file
    for (int i = 0; i < 100; i++) {
    course_t course;
    int bytes_read = read(course_db_fd, &course, sizeof(course));
    if (bytes_read < 0) {
        perror("read");
        break;
    }

    if (bytes_read == 0) {
        break;
    }

    // Unlock the record for the current course
    // TODO: Record locking
    // fcntl(course_db_fd, F_UNLCK, &course_db_lock, &course_db_lock.l_type, course.id * sizeof(course_t));

    // Add the course to the database
    course_db[course.id] = course;
}

// Unlock the course database file
// fcntl(course_db_fd, F_UNLCK, &course_db_lock);

// Close the course database file
close(course_db_fd);
}

void save_courses_db() {
    // Open the course database file
    int course_db_fd = open("courses.txt", O_RDWR | O_CREAT, 0644);
    if (course_db_fd < 0) {
    perror("open");
    exit(1);
    }
    // TODO: File locking
    // Lock the course database file
    // fcntl(course_db_fd, F_SETLK, &course_db_lock);

    // Write each course to the file
    for (int i = 0; i < 100; i++) {
    course_t course = course_db[i];
    if (course.name[0] != '\0') {
        // Lock the record for the current course
        // TODO: Record locking
        // fcntl(course_db_fd, F_SETLK, &course_db_lock, &course_db_lock.l_type, course.id * sizeof(course_t));

        // Write the course to the file
        int bytes_written = write(course_db_fd, &course, sizeof(course));
        if (bytes_written < 0) {
            perror("write");
            break;
        }

        // Unlock the record for the current course
        // fcntl(course_db_fd, F_UNLCK, &course_db_lock);
    }
}

    // Unlock the course database file
    // fcntl(course_db_fd, F_UNLCK, &course_db_lock);

    // Close the course database file
    close(course_db_fd);
}


int create_course(char *name, int credits, char *instructor) {
    // TODO: Create load_course_db()
    int course_db_fd = open("/home/yash/ss_mini_project/db/courses.db", O_RDWR | O_CREAT, 0644);
    if (course_db_fd < 0) {
        perror("open");
        exit(1);
    }


    // TODO: File locking
     // Lock the course database file
    // fcntl(course_db_fd, F_SETLK, &course_db_lock);

    // Find the next available course ID
    int id = 0;
    while (course_db[id].name[0] != '\0') {
        id++;
    }

    // Create a new course
    course_t course = { id, name, credits, instructor }; // ? use strcpy() for strings
    course_db[id] = course;


    // Unlock the course database file
    // fcntl(course_db_fd, F_UNLCK, &course_db_lock);

    // Return the new course ID
    return id;
}

void update_course(int id, char *name, int credits, char *instructor) {
    int course_db_fd = open("/home/yash/ss_mini_project/db/courses.db", O_RDWR | O_CREAT, 0644);
    if (course_db_fd < 0) {
        perror("open");
        exit(1);
    }

    // Lock the course database file
    // fcntl(course_db_fd, F_SETLK, &course_db_lock);

    // Get the course to be updated
    course_t course = course_db[id];

    // Update the course information
    // course.name = name; // ? use strcpy() for strings
    course.credits = credits;
    // course.instructor = instructor; // ? use strcpy() for strings

    // Save the updated course to the database
    course_db[id] = course;

    // Unlock the course database file
    // fcntl(course_db_fd, F_UNLCK, &course_db_lock);
}

void delete_course(int id) {
    // Lock the course database file
    // fcntl(course_db_fd, F_SETLK, &course_db_lock);

    // TODO: File locking
    // Get the course to be deleted
    course_t course = course_db[id];

    // Set the course's name to an empty string
    course.name[0] = '\0';

    // Unlock the course database file
    // fcntl(course_db_fd, F_UNLCK, &course_db_lock);
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
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client_connection, client_connection);
    }
    // -----
    close(server_socket);
}