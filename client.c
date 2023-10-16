#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<termios.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#define SERVER_PORT 8888

void main(void) {
    struct termios old_settings, new_settings;
    tcgetattr(STDIN_FILENO, &old_settings);
    char username[100];
    char password[100];
    // Create a socket
    int client_socket=socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connect_result = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if(connect_result < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char message[1024];
    int recv_result = recv(client_socket, message, sizeof(message), 0);
    if (recv_result < 0) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", message);
    char choice[1];
    char* main_menu="Welcome to Academia\n"
                        "1. Admin\n"
                        "2. Faculty\n"
                        "3. Student\n"
                        "Enter your Choice : ";
    printf("%s", main_menu);
    scanf("%s", choice);
    if(send(client_socket, choice, sizeof(choice), 0) < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }
    printf("Enter your username : ");
    scanf("%s", &username);
    if(send(client_socket, username, sizeof(username), 0) < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }
    new_settings = old_settings;
    new_settings.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    printf("Enter your password : ");
    scanf("%s", password);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
    if(send(client_socket, password, sizeof(password), 0) < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }
    printf("\n", password);
    
    char msg[1024];
    if((recv(client_socket, msg, sizeof(msg), 0))<0) {
        perror("recv");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    printf("%s\n", msg);
    int ch=atoi(choice);
    if(ch == 1) {
        char mch[1];
        char* menu="\nWelcome, Admin\n"
                        "1. Add Student\n"
                        "2. Add Faculty\n"
                        "3. Activate/Deactivate Student\n"
                        "4. Update Student/Faculty details\n"
                        "5. Exit\n"
                        "Enter your Choice : ";
        printf("%s", menu);
        scanf("%s", mch);
        if(send(client_socket, mch, sizeof(mch), 0) < 0) {
            perror("send");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
        if(atoi(mch)==1) {
            char u[100]={0};
            char pass[100]={0};
            char type[1];
            printf("Enter username: ");
            scanf("%s", u);
            if(send(client_socket, u, sizeof(u), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
            printf("Enter password: ");
            scanf("%s", pass);
            if(send(client_socket, pass, sizeof(pass), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
            printf("Enter user type: ");
            scanf("%s", type);
            if(send(client_socket, type, sizeof(type), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
        }
    }
}