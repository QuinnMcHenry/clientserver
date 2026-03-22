#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 8080 // port to connect to on server

int main(int argc, char const *argv[])
{
    int sock = 0; // socket id
    int valread; // amount of bytes to read from server
    struct sockaddr_in serv_addr; // will hold server address info
    char buffer[1024] = {0}; // empty buffer to hold server response
    char message[1024]; // buffer to hold messages

    // attempt to create socket, returns socket ID, negative if it failed
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_port = htons(PORT); // set server addr port to 8080


    // write localhost as the server addr address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // attempt connection to server using socket and server info.
    // cast to struct sockaddr bc connect() expects a type
    // returns negative if failed
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // send this client's PID to the server so it knows which client we are
    pid_t pid = getpid();
    send(sock, &pid, sizeof(pid_t), 0);
    printf("Client 2 connected with PID: %d\n", pid);

    // wait to hear if the server has both clients connected
    memset(buffer, 0, 1024); // clears the buffer before each read
    valread = read(sock, buffer, 1024);
    printf("Server: %s\n", buffer);

    // if server says only one client is up, exit
    if (strncmp(buffer, "only one client is up", 21) == 0)
    {
        close(sock);
        return -1;
    }

    // keep sending and receiving until we type BYE
    while (1)
    {
        // read message from user, cannot use scanf because it will stop at the
	// first space. we use fgets() to read the whole line, but it also
	// gets newline characters, so use strcspn() to replace those with
	// null terminator.
        printf("Client 2 - Enter message: ");
        fgets(message, 1024, stdin);
	message[strcspn(message, "\n")] = 0;

        // send message to server
        send(sock, message, strlen(message), 0);

        // if we said BYE, stop
        if (strcmp(message, "BYE") == 0)
        {
            printf("Disconnecting...\n");
            break;
        }

        // wait for the forwarded message from client 1 via server
        memset(buffer, 0, 1024); // clear before read
        valread = read(sock, buffer, 1024);
        printf("Client 1 says: %s\n", buffer);

        // if client 1 said BYE, stop
        if (strcmp(buffer, "BYE") == 0)
        {
            printf("Client 1 has disconnected. Exiting...\n");
            break;
        }
    }

    close(sock);
    return 0;
}
