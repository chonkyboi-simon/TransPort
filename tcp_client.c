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

int get_socket_and_connect_as_clinet(char *server_ip, char *server_port)
{
	int sock;						// Socket descriptor
	struct sockaddr_in address_server;	// server address

    //Create a socket: IPv4(PF_INET), reliable(SOCK_STREAM), TCP protocol(IPPROTO_TCP)
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return -1;//Failed to create socket
    //fcntl(sock,F_SETFL,O_NONBLOCK);   //set socket to non-block

	//Construct the server address structure
    memset(&address_server, 0, sizeof(address_server));         /* Zero out structure */
    address_server.sin_family      = AF_INET;                   /* Internet address family */
    address_server.sin_addr.s_addr = inet_addr(server_ip);      /* Server IP address */
    address_server.sin_port        = htons(atoi(server_port));  /* Server port */

    //Establish the connection to the Server
    if (connect(sock, (struct sockaddr *) &address_server, sizeof(address_server)) < 0)
	{
		//Failed to connect to server
		return -2;
	}
	else
	{
		//successfully create socket and connect to the DUT
		return sock;
	}
}

int tcp_send_as_clinet(int sock, char *buffer_tx, int size_buffer_tx)
{


	//send data to server
	if (send(sock, buffer_tx, size_buffer_tx, MSG_NOSIGNAL) != size_buffer_tx)
	{
		return -1;
	}
	else
	{
//		//print out sent data
//		printf("\nSent "COLOR_GREEN"%d"COLOR_RESET" bytes of data to server:",size_buffer_tx);
//		printf("\nHEX format:\n"COLOR_GREEN);		//print out in hex format
//		int i=0;
//		while(i!=size_buffer_tx)
//		{
//			printf("%02x ", buffer_tx[i++]);
//			if ((i%16)==0) printf("\n");
//		}
//		printf(COLOR_RESET);

		// printf("\n\tascii format:\t<start>"COLOR_GREEN);	//print out in ascii format
		// i=0;
		// while(i!=size_buffer_tx)
		// {
			// printf("%c", buffer_tx[i++]);
		// }
		// printf(COLOR_RESET"<end>\n");

		return size_buffer_tx;
	}
}

int tcp_receive_as_client(int sock, char *buffer_rx, int size_buffer_rx)
{
	int data_length;

    //receive data from server
    if ((data_length=recv(sock, buffer_rx, size_buffer_rx, 0))<= 0)
    {
        //recv() failed or connection closed prematurely
        return -1;
    }

    if (data_length!=0)//received data
    {
//        //print out received data
//        printf("\nReceived "COLOR_GREEN"%d"COLOR_RESET" bytes of data from server:",data_length);
//        printf("\nHEX format:\n"COLOR_GREEN);//print out in hex format
//        int i=0;
//        while(i!=data_length)
//        {
//            printf("%02x ", buffer_rx[i++]);
//            if (i==16) printf("\n");
//        }
//        printf(COLOR_RESET);

        return data_length;
    }
    return data_length;
}

int tcp_receive_as_client_select(int sock, char *buffer_rx, int size_buffer_rx)
{
	fd_set readfds;                 //define a set of file descriptors (including sockets) to be monitored and read

	struct timeval timeout;         //timeout value for checking socket status
	//and set timeout 0.5 second
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;

	//add socket to a set file descriptor which will be monitored and read from
	FD_ZERO(&readfds);              //clear all entries in the set of file descriptors
	FD_SET(sock,&readfds);          //add the new socket to the set

	int length_in_byte_rx_data;
	int i;

	select(sock+1,&readfds,NULL,NULL,&timeout);//get socket status
	if (FD_ISSET(sock,&readfds))
		{
			//receive data from DUT(Server)
			if ((length_in_byte_rx_data=recv(sock, buffer_rx, size_buffer_rx, 0))<= 0)
			{
				//recv() failed or connection closed prematurely
				return -1;
			}

			if (length_in_byte_rx_data!=0)//received data
			{
				//print out received data
				printf("\nReceived "COLOR_GREEN"%d"COLOR_RESET" bytes of data from DUT:",length_in_byte_rx_data);
				printf("\nHEX format:\n"COLOR_GREEN);//print out in hex format
				i=0;
				while(i!=length_in_byte_rx_data)
				{
					printf("%02x ", buffer_rx[i++]);
					if (i==16) printf("\n");
				}
				printf(COLOR_RESET);
			}
			else
				return 0;
		}
	else
		return -3;//timeout

	return length_in_byte_rx_data;
}
