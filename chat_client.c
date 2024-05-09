// chat_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048

void *receive_message(void *socket_desc) {
    // Receive messages from the server
    int sock = *(int*)socket_desc;
    char server_reply[BUFFER_SIZE];
    while (1) {
        memset(server_reply, 0, BUFFER_SIZE);
        if (recv(sock, server_reply, BUFFER_SIZE, 0) < 0) {
            puts("recv failed");
            break;
        }
        printf("Server reply: %s\n", server_reply);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // Create client socket
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], username[100];
    
    // Connect to the server
    if (argc < 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htsons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror(Connected failed, error.");
        return 1;
    }
    
    // Create a new thread to receive messages
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_message, (void*)&sock);

    // Send messages to the server
    while (1) {
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);

        if (send(sock, message, strlen(message), 0) < 0) {
            puts("Send failed.");
            return 1;
        }
    }
    close(sock);
    return 0;
    //MANY OF THE CHANGES I MADE FOR SERVER AND CLIENT COULD EASILY BE INCORRECT OR UNFINISHED
}
