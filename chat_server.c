// chat_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

// Global variables
int client_sockets[MAX_CLIENTS];
pthread_mutex_t lock;

void *client_handler(void *socket_desc) {
    int new_sock = *(int*)socket_desc;
    char client_message[BUFFER_SIZE];

    // Receive messages from the client
    while (1) {
        memset(client_message, 0, sizeof(client_message));
        if (recv(new_sock, client_message, BUFFER_SIZE, 0) <= 0) {
            break;
        }

        // Write the received message to chat_history
        pthread_mutex_lock(&lock);
        FILE *file = fopen("chat_history", "a");
        fprintf(file, "%s\n", client_message);
        fclose(file);
        pthread_mutex_unlock(&lock);

        // Send the message back to all clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != 0) {
                write(client_sockets[i], client_message, strlen(client_message));
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket, c;
    struct sockaddr_in server, client;
    pthread_t thread_id;

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Could not create socket");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);  // Use the port number passed as an argument

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen
    listen(server_socket, 3);

    // Accept incoming connections
    c = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&c))) {
        // Add the new client's socket to the array
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = client_socket;
                break;
            }
        }

        // Create a new thread for the client
        if (pthread_create(&thread_id, NULL, client_handler, (void*)&client_socket) < 0) {
            perror("Could not create thread");
            return 1;
        }
    }

    return 0;
}
