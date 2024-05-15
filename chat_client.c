// chat_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define BUFFER_SIZE 2048
void report(const char* msg, int terminate) {
    perror(msg);
    if (terminate) exit(-1);
}

// function to recieve messages from server
void *receive_message(void *socket_desc) {

    int sock = *(int*)socket_desc;
    char server_reply[BUFFER_SIZE];

    while (1) {
        memset(server_reply, 0, BUFFER_SIZE);
        int bytes_recieved = recv(sock, server_reply, BUFFER_SIZE, 0);

        if (bytes_recieved <= 0) {
            if (bytes_recieved == 0) {
                printf("Server closed the connnection.\n");
            }
            else {
                perror("recv failed");
            }
            break;
        }

        server_reply[bytes_recieved] = '\0';
        printf("\rServer reply: %s\n", server_reply);
        printf("Enter message (type '/quit' to exit): ");
        fflush(stdout);
    }
    return NULL;
}


// main function
int main(int argc, char *argv[]) {

    // Check for the command arguments
    if (argc < 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    // create the client socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        report("Could not create socket", 1);
    }
    puts("Socket created");

    // connect to the server
    struct sockaddr_in saddr;
    saddr.sin_addr.s_addr = inet_addr(argv[1]);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        close(sock);
        report("Connected failed, error.", 1);
    }

        // Prompt user for username
    char username[100];
    printf("Enter username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        close(sock);
        report("Error reading username", 1);
    }
    username[strcspn(username, "\n")] = 0;

    // Send username to the server
    if (send(sock, username, strlen(username), 0) < 0) {
        perror("Failed to send username");
        return 1;
    }

    // Create a new thread to recieve messages
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_message, (void*)&sock) < 0) {
        close(sock);
        report("Could not create recieve thread", 1);
    }

    // Prompt user for message and send
    char message[BUFFER_SIZE];
    while (1) {
        printf("Enter message: ");
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL) {
            perror("Error reading message");
            break;
        }

        if (strcmp(message, "/quit\n") == 0) {
            break;
        }

        if (send(sock, message, strlen(message), 0) < 0) {
            perror("Send failed");
            break;
        }
    }

    // Exit function
    close(sock);
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);
    puts("Recieve thread finished");
    return 0;
}
