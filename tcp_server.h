#ifndef TCP_SERVER_FUNC
    #define TCP_SERVER_FUNC
    int get_socket_and_listen_as_server(unsigned long *client_ip, char *argv_port);
    int tcp_receive_as_server(int sock, char *buffer_rx, int size_buffer_rx);
    int tcp_send_as_server(int sock, char *buffer_rx, int size_buffer_rx);
#endif // TCP_SERVER_FUNC
