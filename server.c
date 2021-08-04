/*
 * Description: A server will open a socket, it will need to bind an IP address and port to the socket before it can listen for 
 *              connections, once it has binded the socket the server will listen for connections and the last step is to accept the
 *              connections.
 * Steps:
 * 1. Create socket
 * 2. Bind
 * 3. Listen
 * 4. Accept
 * 5. Receive
 * 6. Send
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>       /* memset */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>    /* inet_addr */
#include <netinet/in.h>
#include <unistd.h>       /* close */

#define SIZE 1024

int socket_description(int port, struct sockaddr_in server_addr);

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./server <port>\n\n");
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    //create TCP socket
    int sock_fd = socket_description(port, server_addr);

    
    //accept incoming connections
    fprintf(stdout, "Waiting for new connection....\n");
    socklen_t sock_len = sizeof(server_addr);

    while (1)
    {
        int client = accept(sock_fd,(struct sockaddr *)&client_addr, &sock_len);
        if (client < 0)
        {
            perror("Error accepting incoming connection");
            exit(EXIT_FAILURE);
        }
        
        char const *client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);
        fprintf(stdout, "Accepted new connection on %s:%d\n", client_ip, client_port);

        
        char buffer[SIZE];
        char *p_buffer = buffer;
        int bytes_received;

        
        while((bytes_received = recv(client,p_buffer,SIZE,0) > 0))
        {
            //check if the received message ends in a newline character, replace with null byte
            if (*(p_buffer + bytes_received - 1) == '\n')
            {
                *(p_buffer + bytes_received - 1) = '\0';
            }
            
            fprintf(stdout, "Received message from %s: %s", client_ip, p_buffer);
            int bytes_sent = send(client, p_buffer, bytes_received, 0);
            if (bytes_sent < 0)
            {
                perror("Error receiving message");
                exit(EXIT_FAILURE);
            }
        }
        close(client);     
    }

    return 0;

}

int socket_description(int port, struct sockaddr_in server_addr)
{
    //create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind
    int bind_socket = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_socket < 0)
    {
        perror("Error binding IP address and port");
        exit(EXIT_FAILURE);
    }

    //listen for incoming connections
    int listen_socket = listen(server_socket, 4);
    if (listen_socket < 0)
    {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }
    
    return server_socket;
}