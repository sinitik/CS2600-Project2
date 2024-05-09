// chat_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

// Global variables
int client_sockets[MAX_CLIENTS] = {0};
pthread_mutex_t lock;

void *client_handler(void *socket_desc) {
    int new_sock = *(int*)socket_desc;
    free(socket_desc);
    char client_message[BUFFER_SIZE];

    // Receive messages from the client
    while (1) {
        memset(client_message, 0, sizeof(client_message));
        int read_size = recv(sock, client_message, BUFFER_SIZE, 0);
        if (read_size <= 0) {
            break;
        }

        // Write the received message to chat_history
        pthread_mutex_lock(&lock);
        FILE *file = fopen("chat_history", "a");
        if (file) {
            fprintf(file, "%s\n", client_message);
            fclose(file);
        }
        pthread_mutex_unlock(&lock);

        // Send the message back to all clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != 0) {
                send(client_sockets[i], client_message, strlen(client_message), 0);
            }
        }
    }

    // For removing the disconnected client
    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == sock) {
            client_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    close(sock);
    return 0;
}

int main(int argc, char *argv[]) {
    int server_socket, c, *new_sock;
    struct sockaddr_in server, client;

    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

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
    printf("Waiting for incoming connections...\n");
    pthread_mutex_init(&lock, NULL);

    // Accept incoming connections
    c = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&c))) {
        printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        pthread_t thread_id;
        new_sock = malloc(1);
        *new_sock = client_socket;
        pthread_mutex_lock(&lock);
        
        // Add the new client's socket to the array
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = client_socket;
                break;
            }
        }
        pthread_mutex_unlock(&lock);

        // Create a new thread for the client
        if (pthread_create(&thread_id, NULL, client_handler, (void*)&client_socket) < 0) {
            perror("Could not create thread");
            return 1;
        }
    }

    if (client_socket < 0) {
        perror("Accept Failed);
        return 1;
    }

    pthread_mutex_destroy(&lock);
    close(server_socket);
    return 0;
}
