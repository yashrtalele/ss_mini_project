#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<termios.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#define SERVER_PORT 8888

typedef struct {
    int id;
    char name[128];
    int credits;
    char instructor[128];
    int max_seats;
    int available_seats;
} course_t;

void main(void) {
    struct termios old_settings, new_settings;
    tcgetattr(STDIN_FILENO, &old_settings);
    char username[100]={0};
    char password[100]={0};
    int client_socket=socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
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
    if(strcmp(msg, "Failed to authenticate user!\n")==0) {
        close(client_socket);
        exit(EXIT_FAILURE);
    }
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
        else if(atoi(mch)==2) {
            char u[100]={0};
            char pass[100]={0};
            char type[1];
            printf("Enter faculty username: ");
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
        else if(atoi(mch)==3) {
            char id[10];
            printf("Enter student id: ");
            scanf("%s", id);
            if(send(client_socket, id, sizeof(id), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
            char ch[1];
            char *m="Select - 1. Activate 2. Deactivate : ";
            printf("%s", m);
            scanf("%s", ch);
            if(send(client_socket, ch, sizeof(ch), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }
        else if(atoi(mch)==4) {
            char *m="1. Student 2. Faculty: ";
            printf("%s", m);
            char usr[2];
            scanf("%s", usr);
            if(send(client_socket, usr, sizeof(usr), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            char id[100];
            printf("Enter id: ");
            scanf("%s", id);
            if(send(client_socket, id, sizeof(id), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            char newn[100];
            printf("Enter new name: ");
            scanf("%s", newn);
            if(send(client_socket, newn, sizeof(newn), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }
        else if(atoi(mch) > 4) {
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
    }
    else if(ch == 2) {
        char mch[1];
        char* menu="\nWelcome, Faculty\n"
                        "1. Add new Course\n"
                        "2. Remove offered Course\n"
                        "3. View enrollments in Courses\n"
                        "4. Password Change\n"
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
            char course_name[100]={0};
            char instructor[100]={0};
            char credit[5]={0};
            char max_seats[5]={0};
            printf("Enter course name: ");
            scanf("%s", course_name);
            if(send(client_socket, course_name, strlen(course_name), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
            printf("Enter instructor name: ");
            scanf("%s", instructor);
            if(send(client_socket, instructor, strlen(instructor), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
            printf("Enter credit: ");
            scanf("%s", credit);
            if(send(client_socket, credit, strlen(credit), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
            printf("Enter maximum no. of seats: ");
            scanf("%s", max_seats);
            if(send(client_socket, max_seats, strlen(max_seats), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
        }
        else if(atoi(mch)==2) {
            char cid[5]={0};
            printf("Enter course id to remove: ");
            scanf("%s", cid);
            if(send(client_socket, cid, strlen(cid), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
            write(STDOUT_FILENO, msg, strlen(msg));
        }
        else if(atoi(mch)==3) {
            int num_courses;
            recv(client_socket, &num_courses, sizeof(int), 0);
            course_t *courses = malloc(sizeof(course_t) * num_courses);
            if (courses == NULL) {
                perror("malloc");
                exit(1);
            }
            printf("%d\n", num_courses);
            for (int i = 0; i < num_courses; i++) {
                recv(client_socket, &courses[i], sizeof(course_t), 0);
            }
            for (int i = 0; i < num_courses; i++) {
                printf("Course id: %d\n", courses[i].id);
                printf("Course name: %s\n", courses[i].name);
                printf("Course instructor: %s\n\n", courses[i].instructor);
                printf("Course's maximum seats: %d\n\n", courses[i].max_seats);
                printf("Course's available seats: %d\n\n", courses[i].available_seats);
            }
            free(courses);
        }
        else if(atoi(mch)==4) {
            char pass[128]={0};
            printf("Enter new password: ");
            scanf("%s", pass);
            send(client_socket, pass, sizeof(pass), 0);
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
        }
    }
    else if(ch == 3) {
        char mch[1];
        char* menu="\nWelcome, Student\n"
                        "1. Enroll to new Courses\n"
                        "2. Unenroll from already enrolled Courses\n"
                        "3. View enrolled Courses\n"
                        "4. Password Change\n"
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
            char course_id[5]={0};
            printf("Enter course id: ");
            scanf("%s", course_id);
            if(send(client_socket, course_id, sizeof(course_id), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }
        if(atoi(mch)==2) {
            char course_id[5]={0};
            printf("Enter course id: ");
            scanf("%s", course_id);
            if(send(client_socket, course_id, sizeof(course_id), 0) < 0) {
                perror("send");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }
        else if(atoi(mch)==3) {
            memset(msg, 0, sizeof(msg));
            if(recv(client_socket, msg, sizeof(msg), 0)) {
                perror("recv");
            }
            write(STDOUT_FILENO, msg, strlen(msg));
        }
        else if(atoi(mch)==4) {
            char pass[128]={0};
            printf("Enter new password: ");
            scanf("%s", pass);
            send(client_socket, pass, sizeof(pass), 0);
            memset(msg, 0, sizeof(msg));
            recv(client_socket, msg, sizeof(msg), 0);
        }
        else if(atoi(mch) > 4) {
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
    }

}