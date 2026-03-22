#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define PORT 8080  // the port we will connect to on the server
                   // think of it as a door number on a building
   
int main(int argc, char const *argv[])
{
    int sock = 0;          // this will be our socket "phone line" identifier
    int valread;           // will store how many bytes we read from the server
    
    struct sockaddr_in serv_addr;  // an envelope we will fill with the server's
                                   // address info (IP, port, address family)
    
    char *hello = "Hello from client";  // the message we will send to the server
                                        // must be a pointer because strings are
                                        // sequences of characters in memory
    
    char buffer[1024] = {0};  // empty 1024 byte bucket to hold the server's response
                              // {0} makes sure there is no garbage data in it

    // attempt to create the socket
    // AF_INET = we are using IPv4
    // SOCK_STREAM = we want a reliable two way TCP connection
    // returns an integer identifier for the socket, negative means failure
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;    // tell the envelope we are using IPv4
    serv_addr.sin_port = htons(PORT);  // set the port on the envelope to 8080
                                       // htons converts to network byte order so
                                       // all machines agree on the format
       
    // convert the IP address from human readable "127.0.0.1" to binary
    // and write it into our envelope
    // 127.0.0.1 is localhost - the server is running on the same machine
    // returns <= 0 if the address is invalid
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    // attempt to connect to the server using our socket and filled out envelope
    // cast to struct sockaddr* because connect() is generic and expects a base type
    // same idea as casting to void** in CUDA - just satisfying the function signature
    // returns negative if connection failed
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // send our hello message through the socket to the server
    // strlen(hello) tells send() how many bytes to send
    // final 0 means no special options
    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent\n");  // confirm to console that message was sent

    // wait for the server to respond and read up to 1024 bytes into our buffer
    // valread will hold how many bytes we actually received
    valread = read(sock, buffer, 1024);

    printf("%s\n", buffer);  // print whatever the server sent back

    return 0;  // program finished successfully
               // socket closes automatically when program exits
}
