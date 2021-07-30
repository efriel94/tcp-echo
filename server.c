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
 * 
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

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./server <port>\n\n");
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);

    //create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind
    int bind_socket = bind(server_socket, (struct sockaddr *)&address, sizeof(address));
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
    
    //accept incoming connections
    fprintf(stdout, "Waiting for new connection....\n");
    struct sockaddr_in remote_connection;
    socklen_t remote_connlen = sizeof(server_socket);

    while (1)
    {
        int client = accept(server_socket,(struct sockaddr *)&remote_connection, &remote_connlen);
        if (client < 0)
        {
            perror("Error accepting incoming connection");
            exit(EXIT_FAILURE);
        }
        
        char const *client_ip = inet_ntoa(remote_connection.sin_addr);
        int client_port = ntohs(remote_connection.sin_port);
        fprintf(stdout, "Accepted new connection on %s:%d\n", client_ip, client_port);

        
        char buffer[SIZE];
        char *p_buffer = buffer;
        int bytes_received;
        while((bytes_received = recv(client,p_buffer,SIZE,0) > 0))
        {
            if (*(p_buffer + bytes_received) == '\n')
            {
                *(p_buffer + bytes_received) = '\0';
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