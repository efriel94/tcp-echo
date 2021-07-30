/*
 * Description: Once a client opens a socket it needs to connect to the server socket endpoint and \
                then be able to send/rec data
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>       /* memset */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>    /* inet_addr */
#include <netinet/in.h>
#include <unistd.h>       /* close */


#define SIZE 256


int tcp_socket(int port, char *ip_address_server);


int main(int argc, char * argv[])
{
    if (argc != 3) 
    {
        fprintf(stdout,"Usage: ./client <server-address> <port>\n");
        exit(EXIT_FAILURE);
    }
    
    char *ip_address_server = argv[1]; 
    int port = atoi(argv[2]);

    //create and return TCP socket description
    int socket_description = tcp_socket(port,ip_address_server);

    //hold client and server data
    char buffer[SIZE];
    char *p_buffer = buffer;

    //send message
    memset(buffer, 0, SIZE);
    fprintf(stdout,"> ");
    if(fgets(p_buffer,SIZE,stdin) != NULL)
    {
        int bytes_sent = send(socket_description, p_buffer, SIZE, 0);
        if (bytes_sent < 0)
        {
            perror("Error: Sending message!");
            exit(EXIT_FAILURE);
        }
    }
    else 
    {
        perror("Error: Writing message to buffer\n");
        exit(EXIT_FAILURE);
    }

    //receive message
    //memset(buffer, 0, SIZE);
    int bytes_received;
    while((bytes_received = recv(socket_description, p_buffer, strlen(p_buffer), 0) > 0))
    {
        if (*(p_buffer + bytes_received) == '\n')
        {
            *(p_buffer + bytes_received) = '\0';
        }

        fprintf(stdout,"Server response: %s\n", p_buffer);
        fprintf(stdout,"> ");
        if(fgets(p_buffer,SIZE,stdin) != NULL)
        {
            int bytes_sent = send(socket_description, p_buffer, SIZE, 0);
            if (bytes_sent < 0)
            {
                perror("Error: Sending message!");
                exit(EXIT_FAILURE);
            }
        }
        else 
        {
            perror("Error: Writing message to buffer\n");
            exit(EXIT_FAILURE);
        }
    }

    // if (bytes_received < 0)
    // {
    //     perror("Error: Receiving message\n");
    //     exit(EXIT_FAILURE);
    // } 
    // else 
    // {
    //     fprintf(stdout, "Server response: %s\n", p_buffer);
    // }
    
    close(socket_description);
    
    return 0;
}


int tcp_socket(int port, char *ip_address_server)
{
    //create socket
    int network_socket = socket(AF_INET,SOCK_STREAM,0);
    if(network_socket == -1){
        perror("Error: Can't create socket!\n");
        exit(EXIT_FAILURE);
    }

    //create IPV4 header
    struct sockaddr_in server_address;          
    memset(&server_address, 0, sizeof(server_address));                /* Copy zeros to the socket structure */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip_address_server);

    //connect to server
    int connection_status = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connection_status == -1)
    {
        perror("There was a problem connecting to the remote socket");
        exit(EXIT_FAILURE);
    }

    return network_socket;
}