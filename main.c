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

#include <sys/ioctl.h>
#include <sys/types.h>

#include "tcp_client.h"
#include "tcp_server.h"
//#include "getch.h"

#define BUFFER_SIZE 16384 //the 2 bytes of length field in the TLS record header stipulates that the maximum TLS record length is 16KB (16384 bytes)
//#define RX_BUFFER_SIZE 16384
//#define TX_BUFFER_SIZE 16384
//TLS record types
#define TLS_RECORD_TYPE_CHANGE_CIPHER_SPEC  0x14
#define TLS_RECORD_TYPE_ALERT               0x15
#define TLS_RECORD_TYPE_HANDSHAKE           0x16
#define TLS_RECORD_TYPE_APPLICATION_DATA    0x17

#include "console_text_format.h"

//#define COLOR_RESET			"\033[0m"			/* Reset */
//#define COLOR_BLACK			"\033[30m"			/* Black */
//#define COLOR_RED			"\033[31m"			/* Red */
//#define COLOR_GREEN			"\033[32m"			/* Green */
//#define COLOR_YELLOW		"\033[33m"			/* Yellow */
//#define COLOR_BLUE			"\033[34m"			/* Blue */
//#define COLOR_MAGENTA		"\033[35m"			/* Magenta */
//#define COLOR_CYAN			"\033[36m"			/* Cyan */
//#define COLOR_WHITE			"\033[37m"			/* White */
//#define COLOR_BOLDBLACK		"\033[1m\033[30m"	/* Bold Black */
//#define COLOR_BOLDRED		"\033[1m\033[31m"	/* Bold Red */
//#define COLOR_BOLDGREEN		"\033[1m\033[32m"	/* Bold Green */
//#define COLOR_BOLDYELLOW	"\033[1m\033[33m"	/* Bold Yellow */
//#define COLOR_BOLDBLUE		"\033[1m\033[34m"	/* Bold Blue */
//#define COLOR_BOLDMAGENTA	"\033[1m\033[35m"	/* Bold Magenta */
//#define COLOR_BOLDCYAN		"\033[1m\033[36m"	/* Bold Cyan */
//#define COLOR_BOLDWHITE		"\033[1m\033[37m"	/* Bold White */

struct tls_record_header
{
    int tls_record_type;
    int tls_version;
    int tls_record_length;
};

struct tls_record
{
    struct tls_record_header header;
    char * data;
};

struct tls_record print_TLS_record(char* buffer)
{
    int i=0;
    struct tls_record tls_record;
    //TLS record header
    printf(COLOR_RESET);
    printf("\n-------------------");
    printf("\nStart of TLS record:");
    printf("\nTLS record header:");
    //record type
    printf(COLOR_BOLDCYAN);
        printf("\n%02x ", 0x000000FF&buffer[0]);//indicator for TLS record type
        tls_record.header.tls_record_type=0x000000FF&buffer[0];
        switch (tls_record.header.tls_record_type)
        {
            case TLS_RECORD_TYPE_ALERT:
                printf("\tAlert\n");
                break;
            case TLS_RECORD_TYPE_APPLICATION_DATA:
                printf("\tApplication data\n");
                break;
            case TLS_RECORD_TYPE_CHANGE_CIPHER_SPEC:
                printf("\tChange cipher spec\n");
                break;
            case TLS_RECORD_TYPE_HANDSHAKE:
                printf("\tHandshake\n");
                break;
            default:
                printf(COLOR_RED);
                printf("\tUNKNOWN_TLS_RECORD_TYPE\n");
                printf(COLOR_RESET);
        }
    //tls protocol version
    printf(COLOR_BOLDBLUE);
        printf("%02x %02x ", 0x000000FF&buffer[1], 0x000000FF&buffer[2]);//indicator for TLS version
        tls_record.header.tls_version=((0x000000FF&buffer[1])<<8)|(0x000000FF&buffer[2]);
        switch (tls_record.header.tls_version)
        {
            case 0x0300:
                printf("\tSSL 3.0\n");
                break;
            case 0x0301:
                printf("\tTLS 1.0\n");
                break;
            case 0x0302:
                printf("\tTLS 1.1\n");
                break;
            case 0x0303:
                printf("\tTLS 1.2\n");
                break;
            default:
                printf(COLOR_RED);
                printf("\tUNKNOWN protocol version!\n");
                printf(COLOR_RESET);
        }
    //record data length
    printf(COLOR_BOLDGREEN);
    printf("%02x %02x ", 0x000000FF&buffer[3], 0x000000FF&buffer[4]);//indicator for TLS record data length
    tls_record.header.tls_record_length=(0x000000FF&buffer[4])|((0x000000FF&buffer[3])<<8);
    printf("\tRecord data size: 0x%04x %d", tls_record.header.tls_record_length, tls_record.header.tls_record_length);
    printf(COLOR_RESET);
    //record data
    //Note that in versions of TLS prior to 1.1, there was no IV field, and the last ciphertext block of the previous record (the "CBC residue") was used as the IV.
    printf("\nTLS record data:\n");
    printf(COLOR_BOLDYELLOW);
        tls_record.data=&buffer[5];
        while(i!=tls_record.header.tls_record_length)
        {
            printf("%02x ", 0x000000FF&buffer[5+i++]);
            if ((i%16)==0) printf("\n");
        }
    printf(COLOR_RESET);
    printf("End of TLS record");
    fflush(stdout);
    //return full record length (record header +record)
    return tls_record;
}

void print_usage(char *bin_name)
{
    printf(COLOR_RED"\nError:"COLOR_RESET" argument error.");
    printf("\nUsage: \n%s <server IP address> <client bound port> <server bound port> [h]",bin_name);
    printf("\n\th\tTurn on hold mode and forward packets one by one by pressing ENTER.");
    printf("\n\t\tPackets can be modified by pressing e and ENTER to invoke the Bless hex editor.(sudo apt-get install bless)");
    printf("\n\t\tOnce the editing is finished, save and close the hex editor to forward the packet.\n");
    printf(COLOR_RESET);
}

int main(int argc, char *argv[])
{
    printf("\nTransport by Simon Zhou.\nCompiled on %s %s.\n", __DATE__, __TIME__);

    int sock_server,sock_client;            // Socket descriptor
    char *buffer;
    char key_press;
    unsigned long client_ip=0;              //struct client_addr;
    FILE* fd_tls_record = NULL;

    int data_length;
	int buffer_offset;
	struct tls_record tls_record;

	int num_of_socks_ready;

	//initialize RNG
	int s_time;
	long l_time;
	//use current calendar time as rnd seed and initialize rnd
	l_time=time(NULL);
	s_time=(unsigned)l_time/2;
	srand(s_time);//initialize random number



    //Test number of arguments
    if ((argc!=4) && (argc!=5))
    {
        print_usage(argv[0]);
        exit(1);
    }

    int hold_mode;
    if (argc==5)
    {
        if (*argv[4]=='h')
        {
            hold_mode=1;
            printf("\nHold mode is ON.");
        }
        else
        {
            print_usage(argv[0]);
            exit(1);
        }
    }
    else
    {
        hold_mode=0;
    }

    buffer=malloc(BUFFER_SIZE+1);
    memset(buffer,0,BUFFER_SIZE+1);
//	//create a socket and connect
//	if ((sock=get_socket_and_connect_as_clinet(argv[3],argv[4]))<0)
//	{
//		if (sock==-1)
//			printf(COLOR_RED"\nError:"COLOR_RESET" failed to create socket\n");
//		else
//			printf(COLOR_RED"\nError:"COLOR_RESET" failed to connect to DUT\n");
//		exit(1);
//	}
//	else
//		printf("\nSuccessfully connected to destination at "COLOR_CYAN"%s"COLOR_RESET" port "COLOR_CYAN"%s"COLOR_RESET"as a TCP client",argv[3],argv[4]);
//
//    int ret;
//    ret=tcp_send_as_clinet(sock,"Hello",5);
//    printf("\ntcp send result:%d", ret);
//
//    char *buffer_rx;
//
//    buffer_rx=malloc(16+1);
//    memset(buffer_rx,0,16+1);
//
//    ret=tcp_receive_as_client(sock,buffer_rx,16);
//    while(ret<=0)
//        ret=tcp_receive_as_client(sock,buffer_rx,16+1);
//    printf("\ntcp receive result:%s", buffer_rx);
//
//	// if (fuzzing_blackbox(sock,109070,256)!=0)
//		// printf("\nfuzzing error.");
//
//	//if (blue_bird_P3500_command_fuzzing(sock,50)!=0)
//		//printf("\nfuzzing error.");
//
//	close(sock);
//	printf("\nSocket closed.\n");
    printf("\nWating for connection from a client to trigger a connection to the server");

	//waiting for a client
	if ((sock_server=get_socket_and_listen_as_server(&client_ip,argv[2]))<0)
	{
        printf(COLOR_RED"\nError:"COLOR_RESET" failed to create server socket\n");
		exit(-2);
	}
	else
	{
		printf("\nClient ("COLOR_CYAN"%ld.%ld.%ld.%ld"COLOR_RESET") successfully connected",client_ip&0xFF, (client_ip&0xFF00)>>8, (client_ip&0xFF0000)>>16, (client_ip&0xFF000000)>>24);
    }

    //connect to server
	if ((sock_client=get_socket_and_connect_as_clinet(argv[1],argv[3]))<0)
	{
		if (sock_client==-1)
			printf(COLOR_RED"\nError:"COLOR_RESET" failed to create client socket\n");
		else
			printf(COLOR_RED"\nError:"COLOR_RESET" failed to connect to server\n");
		exit(-1);
	}
	else
		printf("\nSuccessfully connected to remote server at "COLOR_CYAN"%s"COLOR_RESET" port "COLOR_CYAN"%s"COLOR_RESET" as a TCP client",argv[1],argv[3]);


    fd_set readfds;                 //define a set of file descriptors (including sockets) to be monitored and read

	struct timeval timeout;         //timeout value for checking socket status
	//and set timeout 0.5 second
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;


    while (1)
    {
        //add socket to a set file descriptor which will be monitored and read from
        FD_ZERO(&readfds);              //clear all entries in the set of file descriptors
        FD_SET(sock_server,&readfds);   //add the server socket to the set
        FD_SET(sock_client,&readfds);   //add the client socket to the set
        int numfds=(sock_server>sock_client)?(sock_server+1):(sock_client+1);
        num_of_socks_ready=select(numfds,&readfds,NULL,NULL,&timeout);//get socket status
        if ((num_of_socks_ready)&&(num_of_socks_ready>0))
        {
            //at this point, data can be read from at least one socket
            printf("\n\nNumber of socket ready: %d",num_of_socks_ready);
            //check which socket(s) is ready for reading
            if (FD_ISSET(sock_server,&readfds))
            {
                //receive data from client
                if ((data_length=tcp_receive_as_server(sock_server, buffer, BUFFER_SIZE))<= 0)
                {
                    //recv() failed or connection closed prematurely
                    return -3;
                }

                if (data_length!=0)//received data
                {
                    if (hold_mode)  //intercept mode on
                    {
                        //print out received data
                        printf("\nReceived "COLOR_GREEN"%d"COLOR_RESET" bytes of data from client:",data_length);
                        printf("\nData in hex format:\n"COLOR_GREEN);//print out in hex format
                        buffer_offset=0;
                        while(data_length-buffer_offset)
                        {
                            tls_record=print_TLS_record(buffer+buffer_offset);
                            buffer_offset+=tls_record.header.tls_record_length+5;
                            //data_length-=tls_record.header.tls_record_length+5;
                        }
                        printf(COLOR_RESET);
                        //while(getch());
                        printf("\nData captured, press ENTER to forward or e+ENTER to modify.\n");
                        fflush(stdout);
                        key_press=getchar();
                        if (key_press=='e') // edit and release
                        {
                            getchar();
                            fd_tls_record = fopen("tls_record.txt","w+");
                            if(NULL == fd_tls_record)
                            {
                                printf("\nError, open/create TLS record log file!");
                                return 1;
                            }
                            else
                            {
                                printf("\nTLS record log file created.");
                            }
                            fseek(fd_tls_record,0,SEEK_SET);//reposition to start of the file
                            fwrite(buffer,1,data_length,fd_tls_record);
                            fclose(fd_tls_record);

                            //edit file
                            system("sudo bless tls_record.txt");

                            //user may change the tls log file, reload the buffer then send
                            fd_tls_record=fopen("tls_record.txt","r");
                            fseek(fd_tls_record,0,SEEK_END);//position to the end of the file
                            data_length=ftell(fd_tls_record);//ask for postion to tell the size of the modified tls record in the file
                            rewind(fd_tls_record);//seek back to the beginning
                            fread(buffer,1,data_length,fd_tls_record);//load modfied tls record to buffer

                            //print out modified data
                            buffer_offset=0;
                            printf("\nSending modified TLS record\n");
                            printf(COLOR_YELLOW);
                            while(data_length-buffer_offset)
                            {
                                printf("%02x ", 0x000000FF&buffer[buffer_offset++]);
                                if (buffer_offset%16==0) printf("\n");
                            }
                            printf(COLOR_RESET);
                            //forward data
                            tcp_send_as_clinet(sock_client,buffer,data_length);
                            printf("\nData forwarded.");
                            fflush(stdout);
                        }
                        else //direct release
                        {
                            //forward data
                            tcp_send_as_clinet(sock_client,buffer,data_length);
                            printf("\nData forwarded.");
                            fflush(stdout);
                            memset(buffer,0,BUFFER_SIZE+1);
                        }

                    }
                    else    //intercept mode off
                    {
                        tcp_send_as_clinet(sock_client,buffer,data_length);
                        //print out received data
                        printf("\nReceived "COLOR_GREEN"%d"COLOR_RESET" bytes of data from client:",data_length);
                        printf("\nData in hex format:"COLOR_GREEN);//print out in hex format

                        buffer_offset=0;
                        while(data_length-buffer_offset)
                        {
                            tls_record=print_TLS_record(buffer+buffer_offset);
                            buffer_offset+=tls_record.header.tls_record_length+5;
                            //data_length-=tls_record.header.tls_record_length+5;
                        }
                        printf(COLOR_RESET);
                        fflush(stdout);
                        memset(buffer,0,BUFFER_SIZE+1);
                    }
                }
                else
                {
                    printf("\nReceived 0 bytes of data from client.");
                    //return 0;
                }
            }
            else if (FD_ISSET(sock_client,&readfds))
            {
                //receive data from server
                if ((data_length=tcp_receive_as_client(sock_client, buffer, BUFFER_SIZE))<= 0)
                {
                    //recv() failed or connection closed prematurely
                    return -4;
                }

                if (data_length!=0)//received data
                {
                    if (hold_mode)  //intercept mode on
                    {
                        //print out received data
                        printf("\nReceived "COLOR_GREEN"%d"COLOR_RESET" bytes of data from server:",data_length);
                        printf("\nData in hex format:\n"COLOR_GREEN);//print out in hex format
                        buffer_offset=0;
                        while(data_length-buffer_offset)
                        {
                            tls_record=print_TLS_record(buffer+buffer_offset);
                            buffer_offset+=tls_record.header.tls_record_length+5;
                            //data_length-=tls_record.header.tls_record_length+5;
                        }
                        printf(COLOR_RESET);
                        //while(getch());
                        printf("\nData captured, press ENTER to forward or e+ENTER to modify.\n");
                        fflush(stdout);
                        key_press=getchar();
                        if (key_press=='e') // edit and release
                        {
                            getchar();
                            fd_tls_record = fopen("tls_record.txt","w+");
                            if(NULL == fd_tls_record)
                            {
                                printf("\nError, open/create TLS record log file!");
                                return 1;
                            }
                            else
                            {
                                printf("\nTLS record log file created.");
                            }
                            fseek(fd_tls_record,0,SEEK_SET);//reposition to start of the file
                            fwrite(buffer,1,data_length,fd_tls_record);
                            fclose(fd_tls_record);

                            //edit file
                            system("sudo bless tls_record.txt");

                            //user may change the tls log file, reload the buffer then send
                            fd_tls_record=fopen("tls_record.txt","r");
                            fseek(fd_tls_record,0,SEEK_END);//position to the end of the file
                            data_length=ftell(fd_tls_record);//ask for postion to tell the size of the modified tls record in the file
                            rewind(fd_tls_record);//seek back to the beginning
                            fread(buffer,1,data_length,fd_tls_record);//load modfied tls record to buffer

                            //print out modified data
                            buffer_offset=0;
                            printf("\nSending modified TLS record\n");
                            printf(COLOR_YELLOW);
                            while(data_length-buffer_offset)
                            {
                                printf("%02x ", 0x000000FF&buffer[buffer_offset++]);
                                if (buffer_offset%16==0) printf("\n");
                            }
                            printf(COLOR_RESET);
                            //forward data
                            tcp_send_as_server(sock_server,buffer,data_length);
                            printf("\nData forwarded.");
                            fflush(stdout);
                            memset(buffer,0,BUFFER_SIZE+1);
                        }
                        else //direct release
                        {
                            //forward data
                            tcp_send_as_server(sock_server,buffer,data_length);
                            printf("\nData forwarded.");
                            fflush(stdout);
                            memset(buffer,0,BUFFER_SIZE+1);
                        }

                    }
                    else    //intercept mode off
                    {
                        data_length=tcp_send_as_server(sock_server,buffer,data_length);
                        //print out received data
                        printf("\nReceived "COLOR_GREEN"%d"COLOR_RESET" bytes of data from server:",data_length);
                        printf("\nData in hex format:"COLOR_GREEN);//print out in hex format

                        buffer_offset=0;
                        while(data_length-buffer_offset)
                        {
                            tls_record=print_TLS_record(buffer+buffer_offset);
                            buffer_offset+=tls_record.header.tls_record_length+5;
                            //data_length-=tls_record.header.tls_record_length+5;
                        }
                        printf(COLOR_RESET);
                        fflush(stdout);
                        memset(buffer,0,BUFFER_SIZE+1);
                    }
                }
                else
                {
                    printf("\nReceived 0 bytes of data from server.");
                    //return 0;
                }
            }
        }
            //return -3;//timeout
    }
//    int ret;
//    ret=tcp_send_as_server(sock_server,"Hello",6);
//    printf("\ntcp send result:%d", ret);
//
//    ret=tcp_receive_as_server(sock_server,buffer_rx,16);
//    while(ret<=0)
//        ret=tcp_receive_as_server(sock_server,buffer_rx,16+1);
//    printf("\ntcp receive result:%s", buffer_rx);

    exit(0);
}

