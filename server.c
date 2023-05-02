#include "server_functions.h"
#include "udp.h"

struct client {
    int client_id;
    int last_seq_number;
    int last_result;
};

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("expected usage: ./server <port>\n");
        exit(0);
    }
    struct socket s = init_socket(argv[1]);

    struct client call_table[100];
    int num_clients = 0;

    while (1) {
        struct packet_info packet = receive_packet(s);
        for (int i = 0; i < num_clients; i++) {

        }

        break;
    }
}