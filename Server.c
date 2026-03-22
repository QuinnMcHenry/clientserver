// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>

#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, client1, client2;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    // Ignore control-c command
    signal(SIGINT, SIG_IGN);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
       
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; //localhost
    address.sin_port = htons(PORT);
       
    // Binding socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for up to 2 connections
    if (listen(server_fd, 2) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for clients to connect...\n"); 
    
    //Accept the first client 
    if ((client1 = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
    {
        perror("Accept client 1 failed");
        exit(EXIT_FAILURE);
    }
    printf("First client connected.\n");

    //One client is up
    char *message = "only one client is up\n";
    send(client1, message, strlen(message), 0);

    //Accept the second client 
    if ((client2 = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
    {
        perror("Accept client 2 failed");
        exit(EXIT_FAILURE);
    }
    printf("Second client connected.\n");

    //Chat room loop
    while(1) {
        //Send client1 data
        memset(buffer, 0, sizeof(buffer));
        read(client1, buffer, 1024);
        printf("client1: %s", buffer);
        
        //Break loop when client1 says BYE 
        if(strncmp(buffer, "BYE", 3) == 0) {
            break;
        }

        //Send client2 data
        send(client2, buffer, strlen(buffer), 0);

        //read client 2 data
        memset(buffer, 0, sizeof(buffer));
        read(client2, buffer, 1024);
        printf("client2: %s", buffer);
        
        //Break loop when client2 says BYE
        if (strncmp(buffer, "BYE", 3) == 0) {
            break;
        }

        //Send client2 data to client1
        send(client1, buffer, strlen(buffer), 0);
    }

    //Close clients and server
    close(client1);
    close(client2);
    close(server_fd);

    printf("Server has been exited safely.\n");
    return 0;
}
