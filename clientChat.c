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
#define LENGTH 2048
#define NAME_LEN 32


volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

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

void catch_ctrl_c_and_exit(){
    flag = 1;
}


void recv_msg_handler() {
	char message[LENGTH] = {};
  while (1) {
    int receive = recv(sockfd, message, LENGTH, 0);
    if (receive > 0) {
      printf("%s ", message);
      printf("\n");
      str_overwrite_stdout();
    } else if (receive == 0) {
			break;
    } else {
			// -1
		}
		memset(message, 0, sizeof(message));
  }
}

void send_msg_handler() {
  char message[LENGTH] = {};
	char buffer[LENGTH + 1000] = {};

  while(1) {
  	str_overwrite_stdout();
    fgets(message, LENGTH, stdin);
    str_trim_lf(message, LENGTH);

    if (strcmp(message, "exit") == 0) {
			break;
    } else {
      sprintf(buffer, "%s: %s", name,message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

    bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  catch_ctrl_c_and_exit(2);
}


int main(int argc, char **argv){
//port
    int port = atoi(argv[1]);
//nhap ten client
	char *ip = "127.0.0.1";
	signal(SIGINT, catch_ctrl_c_and_exit);
	printf("Nhap ten cua ban: ");
  fgets(name, 32, stdin);
  str_trim_lf(name, strlen(name));

//dieu kien ten user
	if (strlen(name) > 32 || strlen(name) < 2){
		printf("Ten cua ban phai it nhat 2 ky tu va nho hon 32 ky tu.\n");
		return EXIT_FAILURE;
	}

//khoi tao socket
	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);

//Setup client
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}
  //Gui ten client cho server
	send(sockfd, name, 32, 0);
	printf("=== Da ket noi den voi server ===\n");

//tao luong gui
	pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

//tao luong nhan
	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
			printf("\nBye\n");
			break;
    }
	}

	close(sockfd);

	return EXIT_SUCCESS;
}

