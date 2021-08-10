#include <stdlib.h>
#include <stdio.h>
#include <string.h>       /* memset */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>    /* inet_addr */
#include <netinet/in.h>
#include <unistd.h>       /* close */
#include <sys/select.h>   /* select */

#define SIZE 1024         /* 1KB */
#define MAX_FDS 7         /* Max allowed clients on the server is 3
                           * Client socket numbers start at 4 since the first three is used for read,write,error handling 
                           */

int socket_description(int port, struct sockaddr_in server_addr);
void disconnect_client(fd_set temp_fds, struct sockaddr_in client_addr, socklen_t *client_len, int client);


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
    fd_set readfs, temp_readfs;       //create two sets of file descriptors, one to track active connections (readfs) and the other to hold temporary fds (temp_readfs)
    FD_ZERO(&readfs);                 //initialise current sockets to zero
    FD_SET(sock_fd, &readfs);         //add the master socket (file descriptor) to the fd_set

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
        temp_readfs = readfs;

        int ready = select(MAX_FDS, &temp_readfs, NULL, NULL, NULL);
        if (ready < 0)
        {
            perror("Could not read in file descriptor (select error)");
            exit(EXIT_FAILURE);
        }
        

        //loop over the file descriptors in the set to detect if there ready to be read in
        for (int i = 0; i <= MAX_FDS; i++)
        {
            if (FD_ISSET(i, &temp_readfs))
            {
                // this is a new connection to accept on the master socket, set the new connection to the client structure
                if (i == sock_fd)
                {
                    int client = accept(sock_fd,(struct sockaddr *)&client_addr, &server_len);
                    if (client < 0)
                    {
                        perror("Error accepting incoming connection");
                        exit(EXIT_FAILURE);
                    } 
                    char const *client_ip = inet_ntoa(client_addr.sin_addr);
                    int client_port = ntohs(client_addr.sin_port);
                    fprintf(stdout, "Accepted new connection on %s:%d\n", client_ip, client_port);

                    // add the new client to fd_set
                    FD_SET(client, &readfs);
                } 
                    
                // handle existing client
                else 
                {

                    if((bytes_received = recv(i,p_buffer,SIZE,0) > 0))
                    {
                        //check if the received message ends in a newline character, replace with null byte
                        if (*(p_buffer + strlen(p_buffer) - 1) == '\n')
                        {
                            *(p_buffer + strlen(p_buffer) - 1) = '\0';
                        }
                        

                        char const *client_ip = inet_ntoa(client_addr.sin_addr);
                        int client_port = ntohs(client_addr.sin_port);

                        //check if the client sends a quit message to the server
                        if (strcmp(p_buffer,"quit") == 0)
                        {
                            disconnect_client(readfs,client_addr,&client_len, i);
                            FD_CLR(i, &readfs);
                        }
                        //echo back message
                        else
                        {
                            fprintf(stdout, "Echoing message: \"%s\" back to %s:%d\n",p_buffer, client_ip, client_port);
                            int bytes_sent = send(i, p_buffer, strlen(p_buffer), 0);
                            if (bytes_sent < 0)
                            {
                                perror("Error receiving message");
                                exit(EXIT_FAILURE);
                            }  
                        }
                    }
                    
                    // host disconnected
                    else
                    {
                        disconnect_client(readfs, client_addr, &client_len, i);
                        FD_CLR(i, &readfs);
                    }
                }    
            }   
        }
    }

    return 0;

}

/* 
 * Creates a IPv4 TCP/IP socket
 * 
 * port       : Socket will be bind and listen to port number on host
 * server_addr: Struct for describing the server/host address
 * 
 * returns: socket description
 */
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

    //listen for incoming connections, queue after four connections
    int listen_socket = listen(server_socket, 4);
    if (listen_socket < 0)
    {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }
    
    return server_socket;
}

/* 
 * Will disconnect client from server
 * 
 * temp_fds   : The file descriptor set storing client info
 * client_addr: Struct for describing the client address
 * client_len : Length of client address
 * client     : socket description
 * 
 */
void disconnect_client(fd_set temp_fds, struct sockaddr_in client_addr, socklen_t *client_len, int client)
{
    getpeername(client , (struct sockaddr*)&client_addr, client_len);          
    char const *client_ip = inet_ntoa(client_addr.sin_addr);
    uint16_t client_port = ntohs(client_addr.sin_port);
    fprintf(stdout, "Host disconnected: %s:%d\n", client_ip, client_port);
    
    //close socket and remove client from fd_set
    close(client);  
}