all: chat_server chat_client

chat_server: chat_server.c
    gcc -pthread -o chat_server chat_server.c

chat_client: chat_client.c
    gcc -pthread -o chat_client chat_client.c

clean:
    rm -f chat_server chat_client
