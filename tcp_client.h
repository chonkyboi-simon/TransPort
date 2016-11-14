#ifndef TCP_CLIENT_FUNCS
    #define TCP_CLIENT_FUNCS
    int get_socket_and_connect_as_clinet(char *argv_ip, char *argv_port);
    int tcp_send_as_clinet(int sock, char *buffer_tx, int size_buffer_tx);
    int tcp_receive_as_client(int sock, char *buffer_rx, int size_buffer_rx);
#endif // TCP_CLIENT_FUNCS
