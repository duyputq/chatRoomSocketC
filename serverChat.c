#include <sys/socket.h> //thu vien socket
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h> 
#include <signal.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048
static _Atomic unsigned int cli_count =0;
static int uid = 10;
#define NAME_LEN 32


//client structure: gioi han 32 ky tu
typedef struct
{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

//hai ham nay format lai message nhap vao thoi
void str_overwrite_stdout(){
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length){
    for (int i = 0; i < length; i++)
    {
        if (arr[i] == '\n'){
            arr[i] = '\0';
            break;
        }
    }
    
}

//mutex lock: add 1 client vao hang doi
void queue_add(client_t *cl){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i<MAX_CLIENTS; i++){
        if(!clients[i])
        {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

//mutex lock: loai 1 client vao hang doi
void queue_remove(int uid){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i<MAX_CLIENTS; i++){
        if (clients[i])
        {
            if(clients[i]->uid == uid)
            {
                clients[i] == NULL;
                break;
            }
            
        }
        
    }

    pthread_mutex_unlock(&clients_mutex);
}


void print_ip_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
                addr.sin_addr.s_addr & 0xff,
                (addr.sin_addr.s_addr & 0xff00) >> 8,
                (addr.sin_addr.s_addr & 0xff0000) >>16,
                (addr.sin_addr.s_addr & 0xff000000) >>24
    );
}

//gui tin nhan thay cho ham send
void send_message(char *s, int uid){
    pthread_mutex_lock(&clients_mutex);

    for (int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i] ->uid != uid){
                if (write(clients[i]-> sockfd, s, strlen(s)) <0){
                    printf("ERROR: write to descriptor failed\n");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}


void *handle_client(void *arg){
    char buffer[BUFFER_SIZE];
    char name[32];
    int leave_flag =0;

    cli_count++;
    client_t *cli = (client_t *)arg;

    if(recv(cli->sockfd, name, 32, 0) <=0 || strlen(name) < 2 || strlen(name) >= 32-1)
    {
        printf("Client khong nhap ten. \n");
        leave_flag = 1;
    }else{
        strcpy(cli->name, name);
        sprintf(buffer, "%s da tham gia \n", cli->name);
        printf("%s", buffer);
        send_message(buffer, cli->uid);
    }

    bzero(buffer, BUFFER_SIZE);

    while (1)
    {
        if(leave_flag){
            break;
        }

        int receive = recv(cli->sockfd, buffer, BUFFER_SIZE,0);

        if (receive > 0){
            if (strlen(buffer) > 0){
                send_message(buffer, cli->uid);
                str_trim_lf(buffer, strlen(buffer));
                printf("%s -> %s \n", buffer, cli->name);
            }
        } else if (receive == 0 || strcmp(buffer, "exit") == 0){
            sprintf(buffer, "%s da roi khoi phong tro chuyen\n", cli->name);
            printf("%s", buffer);
            send_message(buffer, cli->uid);
            leave_flag = 1;
        } else {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buffer, BUFFER_SIZE);
    }

    close(cli->sockfd);
    queue_remove(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
}


//11:14

int main(int argc, char const *argv[])
{
    if(argc != 2){
        printf("Usuage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

//khai bao cac bien
    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;


//khoi tao socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //khoi tao socket
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //signals
    signal(SIGPIPE, SIG_IGN);

    //opt socet
    setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option));

    
    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    if (listen(listenfd, 10) < 0)
    {
        perror("ERROR: Socket listening failed");
        return EXIT_FAILURE;
    }


//Next
    printf("====Chao mung den voi phong chat====\n");

    while (1)
    {
        socklen_t client = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &client);

//check so luong client
        if((cli_count +1) == MAX_CLIENTS){
            printf("Qua so luong thanh vien. Huy ket noi: ");
            print_ip_addr(cli_addr);
            close(connfd);
            continue;
        }

//client settings
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;

//add client to queue
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

//reduce cpu usage
        sleep(1);
    }

    return EXIT_SUCCESS;
}
