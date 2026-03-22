#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>

#define PORT 8080
#define TIMEOUT 10  // seconds to wait for second client before complaining

int main(int argc, char const *argv[])
{
    int server_fd;
    int client1_sock, client2_sock;
    int valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    pid_t client1_pid, client2_pid;

    // ignore SIGINT (ctrl-c) so the server cannot be killed that way
    signal(SIGINT, SIG_IGN);

    // create the server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }

    // allow the port to be reused immediately after the server stops
    // without this you often get "address already in use" errors on restart
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        printf("setsockopt failed\n");
        return -1;
    }

    // fill out the server address info
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // accept connections on any network interface
    address.sin_port = htons(PORT);

    // bind the socket to the port - claiming port 8080 for this server
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("Bind failed\n");
        return -1;
    }

    // start listening for incoming connections, queue up to 2
    if (listen(server_fd, 2) < 0)
    {
        printf("Listen failed\n");
        return -1;
    }

    printf("Server is up and listening on port %d\n", PORT);

    // accept the first client
    printf("Waiting for client 1...\n");
    if ((client1_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        printf("Accept failed for client 1\n");
        return -1;
    }
    printf("Client 1 connected\n");

    // read client 1's PID
    read(client1_sock, &client1_pid, sizeof(pid_t));
    printf("Client 1 PID: %d\n", client1_pid);

    // wait for client 2 with a timeout
    // use select() to check if a second client connects within TIMEOUT seconds
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);

    // if no second client connected in time, complain to client 1 and wait again
    if (activity == 0)
    {
        printf("Only one client connected. Notifying client 1...\n");
        send(client1_sock, "only one client is up", strlen("only one client is up"), 0);
        close(client1_sock);
        close(server_fd);
        return -1;
    }

    // accept the second client
    printf("Waiting for client 2...\n");
    if ((client2_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        printf("Accept failed for client 2\n");
        return -1;
    }
    printf("Client 2 connected\n");

    // read client 2's PID
    read(client2_sock, &client2_pid, sizeof(pid_t));
    printf("Client 2 PID: %d\n", client2_pid);

    // both clients are connected - tell them both to proceed
    send(client1_sock, "both clients connected, starting session", 
         strlen("both clients connected, starting session"), 0);
    send(client2_sock, "both clients connected, starting session", 
         strlen("both clients connected, starting session"), 0);

    printf("Both clients connected. Forwarding messages...\n");

    // main loop - forward messages between clients until someone sends BYE
    while (1)
    {
        // read message from client 1
        memset(buffer, 0, 1024);
        valread = read(client1_sock, buffer, 1024);
        printf("Client 1 says: %s\n", buffer);

        // forward client 1's message to client 2
        send(client2_sock, buffer, strlen(buffer), 0);

        // if client 1 said BYE, stop
        if (strcmp(buffer, "BYE") == 0)
        {
            printf("Client 1 sent BYE. Closing session.\n");
            break;
        }

        // read message from client 2
        memset(buffer, 0, 1024);
        valread = read(client2_sock, buffer, 1024);
        printf("Client 2 says: %s\n", buffer);

        // forward client 2's message to client 1
        send(client1_sock, buffer, strlen(buffer), 0);

        // if client 2 said BYE, stop
        if (strcmp(buffer, "BYE") == 0)
        {
            printf("Client 2 sent BYE. Closing session.\n");
            break;
        }
    }

    // clean up and close all sockets
    close(client1_sock);
    close(client2_sock);
    close(server_fd);

    return 0;
}
