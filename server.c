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
void remove_newline(char *p);
void disconnect_client(int fd);

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

        int ready = select(FD_SETSIZE, &temp_readfs, NULL, NULL, NULL);
        if (ready < 0)
        {
            perror("Could not read in ready_socket file descriptors (select error)");
            exit(EXIT_FAILURE);
        }
        

        //loop over the file descriptors in the set to detect if there ready to be read in
        for (int i = 0; i < FD_SETSIZE; i++)
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
                    fprintf(stdout, "Accepted new connection on %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    // add the new client to fd_set
                    FD_SET(client, &readfs);
                } 
                    
                // handle existing client
                else 
                {

                    if((bytes_received = recv(i,p_buffer,SIZE,0) > 0)){
                        //check if the received message ends in a newline character, replace with null byte.
                        remove_newline(p_buffer);
                        if (strcmp(p_buffer,"quit") == 0){
                            disconnect_client(i);
                            FD_CLR(i, &readfs);
                            break;
                        } else {
                            fprintf(stdout, "Echoing message: \"%s\" back to %s:%d\n", p_buffer, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                            int bytes_sent = send(i, p_buffer, bytes_received, 0);
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
                        disconnect_client(i);
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
 * port: Socket will be listen on port num  
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
 * Replaces newline char with null nyte
 * p: The client message
 */
void remove_newline(char *p)
{
    while(*p)
    {
        if(*p == '\n'){
            *p = '\0';
        }
        ++p;
    }
}

/* 
 * Close client socket
 * fd: client file descriptor
 */
void disconnect_client(int fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(fd , (struct sockaddr*)&addr, &addr_len);
    fprintf(stdout, "Host disconnected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    close(fd);
}