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
#include <sys/select.h>   /* select */

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

    //set up synchronous i/o for handling multiple clients
    fd_set current_sockets, ready_sockets;     //create two sets of file descriptors to store, one to track our active connection (current_sock) and the other to hold temporary data (ready_sock)
    FD_ZERO(&current_sockets);                 //initialise current sockets to zero
    FD_SET(sock_fd, &current_sockets);         //adds a file descriptor to the current socket set

    //handle client data
    char buffer[SIZE];
    char *p_buffer = buffer;
    int bytes_received;

    //accept incoming connections
    fprintf(stdout, "Waiting for new connections....\n");
    socklen_t server_len = sizeof(server_addr);
    socklen_t client_len = sizeof(client_addr);

    while (1)
    {

        //copy current sock to ready sock to hold temporary data
        ready_sockets = current_sockets;

        int ready = select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL);
        if (ready < 0)
        {
            perror("Could not read in ready_socket file descriptors (select error)");
            exit(EXIT_FAILURE);
        }
        

        //loop over the file descriptors in the set to detect if there ready to be read in
        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {
                if (i == sock_fd)
                {
                    // this is a new connection to accept, set the new connection to the client structure
                    int client = accept(sock_fd,(struct sockaddr *)&client_addr, &server_len);
                    if (client < 0)
                    {
                        perror("Error accepting incoming connection");
                        exit(EXIT_FAILURE);
                    } 
                    char const *client_ip = inet_ntoa(client_addr.sin_addr);
                    int client_port = ntohs(client_addr.sin_port);
                    fprintf(stdout, "Accepted new connection on %s:%d\n", client_ip, client_port);

                    // add the new client to the set
                    FD_SET(client, &current_sockets);
                } 
                else 
                {
                    // handle existing connection
                    if((bytes_received = recv(i,p_buffer,SIZE,0) > 0))
                    {
                        //check if the received message ends in a newline character, replace with null byte
                        if (*(p_buffer + bytes_received) == '\n')
                        {
                            *(p_buffer + bytes_received) = '\0';
                        }
                        char const *client_ip = inet_ntoa(client_addr.sin_addr);
                        int client_port = ntohs(client_addr.sin_port);

                        fprintf(stdout, "Received message from %s:%d %s", client_ip, client_port, p_buffer);
                        int bytes_sent = send(i, p_buffer, bytes_received, 0);
                        if (bytes_sent < 0)
                        {
                            perror("Error receiving message");
                            exit(EXIT_FAILURE);
                        }  
                    }
                    // host disconnected
                    else
                    {
                        getpeername(i , (struct sockaddr*)&client_addr, &client_len);          
                        char const *client_ip = inet_ntoa(client_addr.sin_addr);
                        uint16_t client_port = ntohs(client_addr.sin_port);
                        fprintf(stdout, "Host disconnected: %s:%d\n", client_ip, client_port);
                        
                        //close socket and remove client from fd_set
                        close(i);
                        FD_CLR(i, &current_sockets);
                    }
                }    
            }   
        }
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