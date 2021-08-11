/********************************************
 *  
 * file:    client.c
 * brief:   The program will connect to a server specified by the user
 *          and send messages which will be echoed back
 * author:  Emmet Friel
 * date:    30/07/2021
 * 
 *******************************************/ 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>       /* memset */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>    /* inet_addr */
#include <netinet/in.h>
#include <unistd.h>       /* close */


#define SIZE 1024


int tcp_socket(int port, char *ip_address_server);
void remove_newline(char *p);


int main(int argc, char * argv[])
{
    if (argc != 3) 
    {
        fprintf(stdout,"Usage: ./client <server-address> <server-port>\n");
        exit(EXIT_FAILURE);
    }
    
    char *ip_address_server = argv[1]; 
    int port = atoi(argv[2]);

    //create and return TCP/IP socket description
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

    //while client is connected to server, run indefinitely
    int bytes_received;
    while((bytes_received = recv(socket_description, p_buffer, strlen(p_buffer), 0) > 0))
    {
        remove_newline(p_buffer);
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

/* 
 * Replaces newline char with null nyte in the recieved message
 * p: The client message
 */
void remove_newline(char *p)
{
    while(*p){
        if(*p == '\n'){
            *p = '\0';
        }
        ++p;
    }
}