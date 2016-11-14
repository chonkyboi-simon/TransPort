#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket(), connect(), send(), and recv()
#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include <time.h>       // for time()
#include <fcntl.h>		// for fcntl()

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "console_text_format.h"

int get_socket_and_listen_as_server(unsigned long *client_ip, char *port)
{
    int sock, new_sock;
    unsigned int clilen_addr_length;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    bzero((char *) &server_addr, sizeof(server_addr));
    //create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("\nERROR (server): opening socket");
        return -1;
    }

    //fcntl(sock,F_SETFL,O_NONBLOCK);   //set socket to non-blocking

    //bind socket to server ip address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(port));
    if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("\nERROR: on binding");
        return -2;
    }

    //sleep(1);
    listen(sock,5);

    clilen_addr_length = sizeof(client_addr);

    printf("\nwaiting for client");

    new_sock = accept(sock,(struct sockaddr *) &client_addr, &clilen_addr_length);
    if (new_sock < 0)
    {
        printf("\nERROR: on accept client");
        return -3;
    }
    *client_ip=client_addr.sin_addr.s_addr;
    printf("\nClient connected");
    printf("\nclient ip address: %d.%d.%d.%d\n", client_addr.sin_addr.s_addr&0xFF, (client_addr.sin_addr.s_addr&0xFF00)>>8, (client_addr.sin_addr.s_addr&0xFF0000)>>16, (client_addr.sin_addr.s_addr&0xFF000000)>>24);

    return new_sock;
}

int tcp_receive_as_server(int sock, char *buffer_rx, int size_buffer_rx)
{
    int data_length;
    bzero(buffer_rx,size_buffer_rx);
    data_length = read(sock,buffer_rx,size_buffer_rx);
    if ((data_length < 0)||(data_length>size_buffer_rx))
        printf("\nERROR: reading from socket");
//    else
//        printf("Here is the message: %s\n",buffer_rx);
    return data_length;
}

int tcp_send_as_server(int sock, char *buffer_rx, int size_buffer_rx)
{
    int data_length;
    data_length = write(sock,buffer_rx,size_buffer_rx);
    if (data_length < 0)
    {
        printf("\nERROR: writing to socket");
        return -1;
    }
    else
        return data_length;
}

