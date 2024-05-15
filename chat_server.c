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

// Report error function
void report(const char* msg, int terminate) {
    perror(msg);
    if (terminate) exit(-1); /* failure */
}

    // Receive messages from the client
// function to handle client messages
void *client_handler(void *socket_desc) {
    // Initialize variables
    int new_sock = *(int*)socket_desc;
    free(socket_desc);
    char client_message[BUFFER_SIZE];
    char username[100];

    // Receive username from the client
    memset(username, 0, sizeof(username));
    int read_size = recv(new_sock, username, sizeof(username), 0);
    if (read_size <= 0) {
        report("Failed to receive username", 1);
    }
    username[strcspn(username, "\n")] = 0;  // remove newline character

    // Receive messages from the client
    while (1) {
        memset(client_message, 0, sizeof(client_message));
        read_size = recv(new_sock, client_message, BUFFER_SIZE, 0);
        if (read_size <= 0) {
            if (read_size == 0) {
                report("Client disconnected", 1);
            }
            else {
                perror("recv failed");
            }
            break;
        }

        // Get the current time
        time_t now = time(NULL);
        char timestamp[20];
        strftime(timestamp, 20, "%I:%M:%S %p", localtime(&now));  // 12-hour format

        // Add username and timestamp to the message
        char message_with_username[BUFFER_SIZE + 200];
        sprintf(message_with_username, "%s [%s]: %s", username, timestamp, client_message);

        // Write the received message to chat_history
        pthread_mutex_lock(&lock);
        FILE *file = fopen("chat_history", "a");
        if (file) {
            if (fprintf(file, "%s\n", message_with_username) < 0) {
                perror("Failed to write to file");
            }
            fclose(file);
        }
        else {
            perror("Failed to open file");
        }
        pthread_mutex_unlock(&lock);

        // Send the message back to all clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != 0) {
                if (send(client_sockets[i], message_with_username, strlen(message_with_username), 0) < 0) {
                    perror("Failed to send message to client");
                }
            }
        }
    }

    // For removing the disconnected client
    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == new_sock) {
            client_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    close(new_sock);
    return NULL;
}


// main function
int main(int argc, char *argv[]) {
    // Checks if the commnad arguments are correct
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        report("Not enough arguments", 1);
    }

    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        report("Could not create socket", 1);
    }

    // Prepare the server address structure
    struct sockaddr_in saddr;  // server address
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(atoi(argv[1]));  // Use the port number passed as an argument

    // Bind
    if (bind(server_socket, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        report("Bind failed", 1);
    }

    // Listen
    if (listen(server_socket, 3) < 0) {
        report("Listen failed", 1);
    }
    printf("Waiting for incoming connections...\n");

    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("Mutex init failed");
            return 1;
    }

    // Initialize variables to accept
    struct sockaddr_in caddr;   // client address
    int len_c = sizeof(struct sockaddr_in);
    int client_socket;
    int *new_sock;

    // Accept client connection
    while ((client_socket = accept(server_socket, (struct sockaddr *)&caddr, (socklen_t*)&len_c))) {
        printf("Connection accepted from %s:%d\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
        pthread_t thread_id;
        new_sock = malloc(sizeof(int));
        if (new_sock == NULL) {
            report("Malloc failed", 0);
            continue;
        }
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
            report("Could not create thread", 1);
        }
    }

    // Clean up
    if (client_socket < 0) {
        report("Accept Failed", 0);
    }

    pthread_mutex_destroy(&lock);
    close(server_socket);
    return 0;
