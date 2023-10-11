#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#define SERVER_PORT 8888

int main() {
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
    close(client_socket);
}