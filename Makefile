server:
        gcc -pthread -o chat_server chat_server.c

client:
        gcc -pthread -o chat_client chat_client.c

clean:
        rm -f chat_client; rm -f chat_server; rm -f chat_history