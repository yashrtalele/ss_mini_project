#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdbool.h>
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
    int active;
} user_t;

typedef struct {
    int id;
    enum user_type type;
    char username[128];
    char password[128];
    int active;
} admin_t;

typedef struct {
    int id;
    enum user_type type;
    char username[128];
    char password[128];
    int active;
} faculty_t;

typedef struct {
    int socket_fd;
    user_t user;
} client_connection_t;

typedef struct {
    int id;
    char name[128];
    int credits;
    char instructor[128];
    int max_seats;
    int available_seats;
} course_t;

typedef struct student {
    int id;
    int num_courses;
    course_t *enrolled_courses[4];
} student_t;

pthread_mutex_t user_db_mutex;
pthread_mutex_t student_lock;

user_t user_db[100];
admin_t admin_db[100];
faculty_t faculty_db[100];

course_t course_db[100];
student_t student_db[100];

void load_admin_db() {
    FILE *fp = fopen("/home/yash/ss_mini_project/db/admin.db", "r");
    if (fp < 0) {
        perror("open");
        exit(1);
    }
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        admin_t user;
        char t[1];
        sscanf(line, "%d %d %s %s %s\n", &user.id, &user.type, user.username, user.password, t);
        user.active = atoi(t);
        admin_db[user.id] = user;
    }
    fclose(fp);
}

void load_faculty_db() {
    FILE *fp = fopen("/home/yash/ss_mini_project/db/faculty.db", "r");
    if (fp < 0) {
        perror("open");
        exit(1);
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        faculty_t user;
        char t[1];
        sscanf(line, "%d %d %s %s %s\n", &user.id, &user.type, user.username, user.password, t);
        user.active = atoi(t);
        faculty_db[user.id] = user;
    }
    fclose(fp);
}

void load_user_db() {
    FILE *fp = fopen("/home/yash/ss_mini_project/db/users.db", "r");
    if (fp < 0) {
        perror("open");
        exit(1);
    }
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {   
        user_t user;
        char t[1];
        sscanf(line, "%d %d %s %s %s\n", &user.id, &user.type, user.username, user.password, t);
        user.active = atoi(t);
        user_db[user.id] = user;
    }
    fclose(fp);
}

void load_sc_db() {
    FILE *fp = fopen("/home/yash/ss_mini_project/db/student_courses.db", "r");
    if (fp < 0) {
        perror("open");
        exit(1);
    }
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        student_t user;
        sscanf(line, "%d %d\n", &user.id, &user.num_courses);
        for(int i=0; i<user.num_courses; i++) {
            fgets(line, sizeof(line), fp);
            sscanf(line, "%d\n", user.enrolled_courses[i]->id);
        }
        student_db[user.id] = user;
    }
    fclose(fp);
}

void save_faculty_db() {
    int fd = open("/home/yash/ss_mini_project/db/faculty.db", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    char buf[1024];
    for (int i = 0; i < 100; i++) {
        faculty_t user = faculty_db[i];
        if (user.username[0] != '\0') {
            sprintf(buf, "%d %d %s %s %d\n", user.id, user.type, user.username, user.password, user.active);
            int bytes_written = write(fd, buf, strlen(buf));
            if (bytes_written < 0) {
                perror("write");
                break;
            }
        }
    }
    close(fd);
}

void save_user_db() {
    int fd = open("/home/yash/ss_mini_project/db/users.db", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    char buf[1024];
    for (int i = 0; i < 100; i++) {
        user_t user = user_db[i];
        if (user.username[0] != '\0') {
            sprintf(buf, "%d %d %s %s %d\n", user.id, user.type, user.username, user.password, user.active);
            int bytes_written = write(fd, buf, strlen(buf));
            if (bytes_written < 0) {
                perror("write");
                break;
            }
        }
    }
    close(fd);
}

void save_sc_db() {
    int fd_s=open("db/student_courses.db", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_s < 0) {
        perror("open");
        exit(1);
    }
    char buf_s[1024];
    for (int i = 0; i < 100; i++) {
        student_t user = student_db[i];
            char snum[5]={0};
            sprintf(buf_s, "%d %d \n", user.id, user.num_courses);
            int bytes_written = write(fd_s, buf_s, strlen(buf_s));
            if (bytes_written < 0) {
                perror("write");
                break;
            }
            for (int i = 0; i < user.num_courses; i++) {
                sprintf(buf_s, "%d\n", user.enrolled_courses[i]->id);
                bytes_written = write(fd_s, buf_s, strlen(buf_s));
                if (bytes_written < 0) {
                    perror("write");
                    break;
                }
            }
    }
    close(fd_s);
}

int authenticate_admin(char *username, char *password) {
    pthread_mutex_lock(&user_db_mutex);
    int i, id;
    for (i = 0; i < 100; i++) {
        admin_t user = admin_db[i];
        if (user.active == 1 && strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
            id=user.id;
            break;
        }
    }
    pthread_mutex_unlock(&user_db_mutex);
    return (i < 100) ? id : -1;
}

int authenticate_faculty(char *username, char *password) {
    pthread_mutex_lock(&user_db_mutex);
    int i, id;
    for (i = 0; i < 100; i++) {
        faculty_t user = faculty_db[i];
        if (user.active == 1 && strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
            id=user.id;
            break;
        }
    }
    pthread_mutex_unlock(&user_db_mutex);
    return (i < 100) ? id : -1;
}

int authenticate_user(char *username, char *password) {
    pthread_mutex_lock(&user_db_mutex);
    int i, id;
    for (i = 0; i < 100; i++) {
        user_t user = user_db[i];
        if (user.active == 1 && strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
            id=user.id;
            break;
        }
    }
    pthread_mutex_unlock(&user_db_mutex);
    return (i < 100) ? id : -1;
}

int create_faculty(char *username, char *password, enum user_type type) {
    pthread_mutex_lock(&user_db_mutex);
    int id = 0;
    while (faculty_db[id].username[0] != '\0') {
        id++;
    }
    faculty_t user;
    user.id=id;
    user.type=type;
    strcpy(user.username, username);
    strcpy(user.password, password);
    user.active=1;
    faculty_db[id]=user;
    printf("%s %s \n", user.username, user.password);
    pthread_mutex_unlock(&user_db_mutex);
    printf("id = %d", id);
    return id;
}


int create_user(char *username, char *password, enum user_type type) {
    pthread_mutex_lock(&user_db_mutex);
    int id = 0;
    while (user_db[id].username[0] != '\0') {
        id++;
    }
    user_t user;
    user.id=id;
    user.type=type;
    strcpy(user.username, username);
    strcpy(user.password, password);
    user.active=1;
    user_db[id]=user;
    printf("%s %s \n", user.username, user.password);
    pthread_mutex_unlock(&user_db_mutex);
    printf("id = %d", id);
    return id;
}

user_t *get_student_by_id(int student_id) {
    struct flock student_db_lock;
    student_db_lock.l_type = F_WRLCK;
    student_db_lock.l_whence = SEEK_SET;
    student_db_lock.l_start = 0;
    student_db_lock.l_len = 0; 
    int student_db_fd = open("db/users.db", O_RDWR);
    fcntl(student_db_fd, F_SETLK, &student_db_lock);
    user_t *student = NULL;
    for (int i = 0; i < 100; i++) {
        user_t current_student = user_db[i];
        if (current_student.id == student_id) {
            student = &current_student;
            break;
        }
    }
    fcntl(student_db_fd, F_UNLCK, &student_db_lock);
    return student;
}

void save_student(user_t *student) {
    struct flock student_db_lock;
    student_db_lock.l_type = F_WRLCK;
    student_db_lock.l_whence = SEEK_SET;
    student_db_lock.l_start = 0;
    student_db_lock.l_len = 0; 
    int student_db_fd = open("db/users.db", O_RDWR);
    fcntl(student_db_fd, F_SETLK, &student_db_lock);
    int student_id=student->id;
    user_db[student_id].active=student->active;
    save_user_db();
    fcntl(student_db_fd, F_UNLCK, &student_db_lock);
}

void activate_deactivate_student(int student_id, int activate) {
    user_t *student = get_student_by_id(student_id);
    char snum[2];
    student->active=activate;
    save_student(student);
}

void load_course_db() {
    FILE *fp = fopen("/home/yash/ss_mini_project/db/courses.db", "r");
    if (fp < 0) {
        perror("open");
        exit(1);
    }
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        course_t course;
        sscanf(line, "%d %s %d %s %d %d\n", &course.id, course.name, &course.credits, course.instructor, &course.max_seats, &course.available_seats);
        course_db[course.id] = course;
    }
    fclose(fp);
}

void save_courses_db() {
    int fd = open("/home/yash/ss_mini_project/db/courses.db", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    char buf[1024];
    for (int i = 0; i < 100; i++) {
        course_t course = course_db[i];
        if (course.name[0] != '\0') {
            sprintf(buf, "%d %s %d %s %d %d\n", course.id, course.name, course.credits, course.instructor, course.max_seats, course.available_seats);
            int bytes_written = write(fd, buf, strlen(buf));
            if (bytes_written < 0) {
                perror("write");
                break;
            }
        }
    }
    close(fd);
}

int create_course(char *name, int credits, char *instructor, int max_seats) {
    int course_db_fd = open("/home/yash/ss_mini_project/db/courses.db", O_RDWR | O_CREAT, 0644);
    if (course_db_fd < 0) {
        perror("open");
        exit(1);
    }
    struct flock course_db_lock;
    course_db_lock.l_type = F_WRLCK;
    course_db_lock.l_whence = SEEK_SET;
    course_db_lock.l_start = 0;
    course_db_lock.l_len = 0;
    fcntl(course_db_fd, F_SETLK, &course_db_lock);
    int id = 0;
    while (course_db[id].name[0] != '\0') {
        id++;
    }
    course_t course; 
    course.id=id;
    course.credits=credits;
    strcpy(course.instructor, instructor);
    strcpy(course.name, name);
    course.max_seats=max_seats;
    course.available_seats=max_seats;
    course_db[id]=course;
    fcntl(course_db_fd, F_UNLCK, &course_db_lock);
    return id;
}

void update_course(int id, char *name, int credits, char *instructor) {
    int course_db_fd = open("/home/yash/ss_mini_project/db/courses.db", O_RDWR | O_CREAT, 0644);
    if (course_db_fd < 0) {
        perror("open");
        exit(1);
    }
    course_t course = course_db[id];
    course.credits = credits;
    strcpy(course.name, name);
    strcpy(course.instructor, instructor);
    course_db[id] = course;
}

void delete_course(int id) {
    course_t course = course_db[id];
    write(STDOUT_FILENO, course.name, strlen(course.name));
    memset(course_db[id].name, 0, sizeof(course_db[id].name));
    save_courses_db();
}

int get_num_students() {
    FILE *fp = fopen("db/users.db", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    int num_courses = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        num_courses++;
    }
    fclose(fp);
    return num_courses;
}

int get_num_courses() {
    FILE *fp = fopen("db/courses.db", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    int num_courses = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        num_courses++;
    }
    fclose(fp);
    return num_courses;
}

void get_courses_from_db(course_t *courses, int num_courses) {
    FILE* fp=fopen("db/courses.db", "r");
    for (int i = 0; i < num_courses; i++) {
        char line[1024];
        fgets(line, sizeof(line), fp);
        course_t *course = &courses[i];
        sscanf(line, "%d %s %d %s %d %d\n", &course->id, course->name, &course->credits, course->instructor, &course->max_seats, &course->available_seats);
    }
    fclose(fp);
}

void send_student_courses(int student_id, int client_socket) {
    student_t student=student_db[student_id];
    pthread_mutex_lock(&student_lock);
    char buffer[1024];
    for (int i = 0; i < student.num_courses; i++) {
        course_t *course = student.enrolled_courses[i];
        sprintf(buffer + strlen(buffer), "%d %s\n", course->id, course->name);
    }
    send(client_socket, buffer, strlen(buffer), 0);
    pthread_mutex_unlock(&student_lock);
}

void send_all_courses(int client_fd) {
    int fd = open("db/courses.db", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLK, &lock);
    int num_courses = get_num_courses();
    course_t *courses = malloc(sizeof(course_t) * num_courses);
    if (courses == NULL) {
        perror("malloc");
        exit(1);
    }
    get_courses_from_db(courses, num_courses);
    send(client_fd, &num_courses, sizeof(int), 0);
    for (int i = 0; i < num_courses; i++) {
        send(client_fd, &courses[i], sizeof(course_t), 0);
    }
    fcntl(fd, F_UNLCK, &lock);
    free(courses);
}

int change_user_password(int id, const char *new_password) {
    int fd=open("db/users.db", O_RDWR);
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLK, &lock);
    strcpy(user_db[id].password, new_password);
    save_user_db();
    fcntl(fd, F_UNLCK, &lock);
    return 1;
}

int change_faculty_password(int id, const char *new_password) {
    int fd=open("db/users.db", O_RDWR);
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLK, &lock);
    strcpy(faculty_db[id].password, new_password);
    save_faculty_db();
    fcntl(fd, F_UNLCK, &lock);
    return 1;
}

int update_student_details(int id, const char *new_name) {
    int fd=open("db/users.db", O_RDWR);
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLK, &lock);
    strcpy(user_db[id].username, new_name);
    save_user_db();
    fcntl(fd, F_UNLCK, &lock);
    return 1;
}

int update_faculty_details(int id, const char *new_name) {
    int fd=open("db/users.db", O_RDWR);
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLK, &lock);
    strcpy(faculty_db[id].username, new_name);
    save_faculty_db();
    fcntl(fd, F_UNLCK, &lock);
    return 1;
}

void write_students_to_file(student_t *students) {
    int num_students = get_num_students();
    int fd = open("db/student_courses.db", O_WRONLY | O_TRUNC);
    if (fd < 0) {
        perror("open");
        return;
    }
    int i = 0; 
    while (i < num_students) {
        char buf[1024];
        sprintf(buf, "%d %d\n", i, students[i].num_courses);
        int bytes_written = write(fd, buf, strlen(buf));
        if (bytes_written < 0) {
            perror("write");
            break;
        }
        for (int j = 0; j < students[i].num_courses; j++) {
            char buf_s[1023];
            memset(buf_s, 0, sizeof(buf_s));
            sprintf(buf_s, "%d\n", students[i].enrolled_courses[j]->id);
            int bytes_written = write(fd, buf_s, strlen(buf_s));
            if (bytes_written < 0) {
                perror("write");
                break;
            }
        }
        i++;
    }
    close(fd);
}

void load_students() {
    FILE *fp = fopen("db/student_courses.db", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    int num_students = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        num_students++;
    }
    student_t *students = malloc(sizeof(student_t) * num_students);
    if (students == NULL) {
        perror("malloc");
        exit(1);
    }
    fseek(fp, 0, SEEK_SET);
    int i = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        int id;
        int num_courses;
        sscanf(line, "%d %d\n", &id, &num_courses);
        students[i].id = id;
        students[i].num_courses = num_courses;
        for (int j = 0; j < num_courses; j++) {
        int course_id;
        fgets(line, sizeof(line), fp);
        sscanf(line, "%d\n", &course_id);
        course_t *course = NULL;
        int n_courses = get_num_courses();
        for (int k = 0; k < n_courses; k++) {
            if (course_db[k].id == course_id) {
            course = &course_db[k];
            break;
            }
        }
        if (course == NULL) {
            printf("Failed to find course with ID %d\n", course_id);
            exit(1);
        }
            students[i].enrolled_courses[j] = course;
            student_db[i]=students[i];
        }
        fgets(line, sizeof(line), fp);
        i++;
    }
    fclose(fp);
}

int enroll_student_in_course(int student_id, int course_id) {
    if (student_id < 0 || student_id >= get_num_students()) {
        printf("Student with ID %d does not exist.\n", student_id);
        return -1;
    }
    if (course_id < 0 || course_id >= get_num_courses()) {
        printf("Course with ID %d does not exist.\n", course_id);
        return -1;
    }
    student_t student = student_db[student_id];
    for (int i = 0; i < student.num_courses; i++) {
        if (student.enrolled_courses[i]->id == course_id) {
        printf("The student is already enrolled in the course.\n");
        return -1;
        }
    }
    if (student.num_courses >= 4) {
        printf("The student can only enroll in 4 courses.\n");
        return -1;
    }
    course_t course = course_db[course_id];
    if (course.available_seats == 0) {
        printf("The course is full.\n");
        return -1;
    }
    student.enrolled_courses[student.num_courses] = &course_db[course_id];
    student.num_courses++;
    course.available_seats--;
    student_db[student_id] = student;
    course_db[course_id] = course;
    write_students_to_file(student_db);
    save_courses_db();
    return 0;
}

void unenroll_from_course(int student_id, int course_id) {
    student_t student=student_db[student_id];
    course_t course=course_db[course_id];
    int course_index = -1;
    for (int i = 0; i < student.num_courses; i++) {
        if (student.enrolled_courses[i]->id == course.id) {
            course_index = i;
            break;
        }
    }
    char s[5]={0};
    sprintf(s, "%d", course_index);
    write(STDOUT_FILENO, s, strlen(s));
    if(course_index==0) {
        student.enrolled_courses[0]=NULL;
    }
    for (int i = course_index; i < student.num_courses - 1; i++) {
        student.enrolled_courses[i] = student.enrolled_courses[i + 1];
    }
    student.num_courses--;
    course.available_seats++;
    student_db[student_id] = student;
    course_db[course_id] = course;
    write_students_to_file(student_db);
    save_courses_db();
}

void *handle_client_connection(void *args) {
    client_connection_t *client_connection = (client_connection_t *) args;
    char choice[1];
    if((recv(client_connection->socket_fd, choice, sizeof(choice), 0)) < 0) {
        perror("recv");
        
    }
    int ch=atoi(choice);
    client_connection->user.type=ch-1;
    if(ch-1 == 0) {
        if((recv(client_connection->socket_fd, client_connection->user.username, sizeof(client_connection->user.username), 0)) < 0) {
            perror("recv");
        }
        if((recv(client_connection->socket_fd, client_connection->user.password, sizeof(client_connection->user.password), 0)) < 0) {
            perror("recv");
        }
        int user_id=authenticate_admin(client_connection->user.username, client_connection->user.password);
        if (user_id < 0) {
            if((send(client_connection->socket_fd, "Failed to authenticate user!\n", 30, 0)) < 0) {
                perror("send");
                close(client_connection->socket_fd);
                exit(EXIT_FAILURE);
            }
            close(client_connection->socket_fd);
        }
        send(client_connection->socket_fd, "Login Success!", 15, 0);
        char mch[1];
        if((recv(client_connection->socket_fd, mch, sizeof(mch), 0)) < 0) {
            perror("recv");
        }
        int mch_n=atoi(mch);
        if(mch_n == 1) {
            char u[128]={0};
            if((recv(client_connection->socket_fd, u, sizeof(u), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Username received!", 19, 0);
            char pass[128]={0};
            if((recv(client_connection->socket_fd, pass, sizeof(pass), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Password received!", 19, 0);
            char t[1];
            if((recv(client_connection->socket_fd, t, sizeof(t), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Type received!", 15, 0);
            create_user(u, pass, atoi(t)-1);
            save_user_db();
            // save_sc_db();
        }
        else if(mch_n == 2) {
            char u[128]={0};
            if((recv(client_connection->socket_fd, u, sizeof(u), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Username received!", 19, 0);
            char pass[128]={0};
            if((recv(client_connection->socket_fd, pass, sizeof(pass), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Password received!", 19, 0);
            char t[1];
            if((recv(client_connection->socket_fd, t, sizeof(t), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Type received!", 15, 0);
            create_faculty(u, pass, atoi(t)-1);
            save_faculty_db();
        }
        else if(mch_n==3) {
            char id[100];
            char bool_a[1];
            int activate;
            if((recv(client_connection->socket_fd, id, sizeof(id), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "id received!", 15, 0);
            if((recv(client_connection->socket_fd, bool_a, sizeof(bool_a), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "bool received!", 15, 0);
            int id_n=atoi(id);
            write(STDOUT_FILENO, bool_a, strlen(bool_a));
            if(strcmp(bool_a, "1") == 0) {
                activate=1;
            } else {
                activate=0;
            }
            activate_deactivate_student(id_n, activate);
        }
        else if(mch_n == 4) {
            char usr[2];
            if((recv(client_connection->socket_fd, usr, sizeof(usr), 0)) < 0) {
                perror("recv");
            }
            int uu=atoi(usr);
            char id[100];
            if((recv(client_connection->socket_fd, id, sizeof(id), 0)) < 0) {
                perror("recv");
            }
            char newn[100];
            if((recv(client_connection->socket_fd, newn, sizeof(newn), 0)) < 0) {
                perror("recv");
            }
            if(uu==1) {
                update_student_details(atoi(id), newn);
            }
            else if(uu==2) {
                update_faculty_details(atoi(id), newn);
            }
        }
    }
    else if(ch-1 == 1) {
        if((recv(client_connection->socket_fd, client_connection->user.username, sizeof(client_connection->user.username), 0)) < 0) {
            perror("recv");
        }
        if((recv(client_connection->socket_fd, client_connection->user.password, sizeof(client_connection->user.password), 0)) < 0) {
            perror("recv");
        }
        int user_id=authenticate_faculty(client_connection->user.username, client_connection->user.password);
        if (user_id < 0) {
            if((send(client_connection->socket_fd, "Failed to authenticate user!\n", 30, 0)) < 0) {
                perror("send");
                exit(EXIT_FAILURE);
            }
            close(client_connection->socket_fd);
        }
        send(client_connection->socket_fd, "Login Success!", 15, 0);
        char mch[1];
        if((recv(client_connection->socket_fd, mch, sizeof(mch), 0)) < 0) {
            perror("recv");
        }
        int mch_n=atoi(mch);
        if(mch_n==1) {
            char course_name[100];
            if((recv(client_connection->socket_fd, course_name, sizeof(course_name), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Course name received!", 22, 0);
            char instructor[100];
            if((recv(client_connection->socket_fd, instructor, sizeof(instructor), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Instructor name received!", 26, 0);
            char credit[2];
            if((recv(client_connection->socket_fd, credit, sizeof(credit), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Credits received!", 18, 0);
            char max_seats[5];
            if((recv(client_connection->socket_fd, max_seats, sizeof(max_seats), 0)) < 0) {
                perror("recv");
            }
            send(client_connection->socket_fd, "Max. seats received!", 21, 0);
            write(STDOUT_FILENO, course_name, strlen(course_name));
            write(STDOUT_FILENO, instructor, strlen(instructor));
            write(STDOUT_FILENO, credit, strlen(credit));
            write(STDOUT_FILENO, max_seats, strlen(max_seats));
            create_course(course_name, atoi(credit), instructor, atoi(max_seats));
            save_courses_db();
        }
        else if(mch_n==2) {
            char course_id[5]={0};
            if(recv(client_connection->socket_fd, course_id, sizeof(course_id), 0)) {
                perror("recv");
            }
            write(STDOUT_FILENO, course_id, strlen(course_id));
            delete_course(atoi(course_id));
            send(client_connection->socket_fd, "Course Deletion Successful!", 28, 0);
        }
        else if(mch_n==3) {
            send_all_courses(client_connection->socket_fd);
        }
        else if(mch_n==4) {
            int student_id=user_id;
            char new_pass[128]={0};
            if(recv(client_connection->socket_fd, new_pass, sizeof(new_pass), 0)) {
                perror("recv");
            }
            send(client_connection->socket_fd, "New Password received!", 21, 0);
            change_faculty_password(student_id, new_pass);
        }
    }
    else if(ch-1 == 2) {    
        if((recv(client_connection->socket_fd, client_connection->user.username, sizeof(client_connection->user.username), 0)) < 0) {
            perror("recv");
        }
        if((recv(client_connection->socket_fd, client_connection->user.password, sizeof(client_connection->user.password), 0)) < 0) {
            perror("recv");
        }
        int user_id=authenticate_user(client_connection->user.username, client_connection->user.password);
        if (user_id < 0) {
            if((send(client_connection->socket_fd, "Failed to authenticate user!\n", 30, 0)) < 0) {
                perror("send");
                exit(EXIT_FAILURE);
            }
            close(client_connection->socket_fd);
        }
        send(client_connection->socket_fd, "Login Success!", 15, 0);
        char mch[1];
        if((recv(client_connection->socket_fd, mch, sizeof(mch), 0)) < 0) {
            perror("recv");
        }
        int mch_n=atoi(mch);
        if(mch_n==1) {
            char course_id[5]={0};
            recv(client_connection->socket_fd, course_id, sizeof(course_id), 0);
            int cnum=atoi(course_id);
            enroll_student_in_course(user_id, cnum);
        }
        else if(mch_n==2) {
            char course_id[5]={0};
            recv(client_connection->socket_fd, course_id, sizeof(course_id), 0);
            int cnum=atoi(course_id);
            unenroll_from_course(user_id, cnum);
        }
        else if(mch_n==3) {
            send_student_courses(user_id, client_connection->socket_fd);
        }
        else if(mch_n==4) {
            int student_id=user_id;
            char new_pass[128]={0};
            if(recv(client_connection->socket_fd, new_pass, sizeof(new_pass), 0)) {
                perror("recv");
            }
            send(client_connection->socket_fd, "New Password received!", 21, 0);
            change_user_password(student_id, new_pass);
        }
    }
}

void main(void) {
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
    load_admin_db();
    load_faculty_db();
    load_user_db();
    load_course_db();
    load_students();
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
        }
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client_connection, client_connection);
    }
    close(server_socket);
}