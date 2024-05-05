// chat_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 2048

void *receive_message(void *socket_desc) {
    // Receive messages from the server
}

int main(int argc, char *argv[]) {
    // Create client socket
    // Connect to the server
    // Create a new thread to receive messages
    // Send messages to the server
}
