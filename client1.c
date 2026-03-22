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
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // send our PID to the server so it knows which client we are
    pid_t pid = getpid();
    send(sock, &pid, sizeof(pid_t), 0);
    printf("Client 1 connected with PID: %d\n", pid);

    // wait to hear if the server has both clients connected
    memset(buffer, 0, 1024);
    valread = read(sock, buffer, 1024);
    printf("Server: %s\n", buffer);

    // if server says only one client is up, exit
    if (strncmp(buffer, "only one client is up", 21) == 0)
    {
        close(sock);
        return -1;
    }

    // main loop - keep sending and receiving until we type BYE
    while (1)
    {
        // read message from user with scanf
        printf("Client 1 - Enter message: ");
        scanf("%s", message);

        // send message to server
        send(sock, message, strlen(message), 0);

        // if we said BYE, stop
        if (strcmp(message, "BYE") == 0)
        {
            printf("Disconnecting...\n");
            break;
        }

        // wait for the forwarded message from client 2 via server
        memset(buffer, 0, 1024);
        valread = read(sock, buffer, 1024);
        printf("Client 2 says: %s\n", buffer);

        // if client 2 said BYE, stop
        if (strcmp(buffer, "BYE") == 0)
        {
            printf("Client 2 has disconnected. Exiting...\n");
            break;
        }
    }

    close(sock);
    return 0;
}
