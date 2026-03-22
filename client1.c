#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 8080

int main(int argc, char const *argv[])
{
    int sock = 0;
    int valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[1024];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid address/ Address not supported\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed\n");
        return -1;
    }

    // read the server's initial status message and just print it
    // server sends "only one client is up" right away - we just acknowledge it
    memset(buffer, 0, 1024);
    valread = read(sock, buffer, 1024);
    printf("Server says: %s\n", buffer);

    // main loop
    while (1)
    {
        // client 1 always sends first
        printf("Client 1 - Enter message: ");
        fgets(message, 1024, stdin);
        message[strcspn(message, "\n")] = 0;

        send(sock, message, strlen(message), 0);

        if (strncmp(message, "BYE", 3) == 0)
        {
            printf("Disconnecting...\n");
            break;
        }

        // wait for client 2's response forwarded by the server
        memset(buffer, 0, 1024);
        valread = read(sock, buffer, 1024);
        printf("Client 2 says: %s\n", buffer);

	// if cli 2 said BYE then exit
        if (strncmp(buffer, "BYE", 3) == 0)
        {
            printf("Client 2 has disconnected. Exiting...\n");
            break;
        }
    }

    close(sock);
    return 0;
}
