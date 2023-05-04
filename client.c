#include "client.h"
#include <stdlib.h>
#include "serialize_structs.h"

struct rpc_connection RPC_init(int src_port, int dst_port, char dst_addr[])
{
    struct rpc_connection connection;
    connection.recv_socket = init_socket(src_port);
    populate_sockaddr(AF_INET, dst_port, dst_addr, &connection.dst_addr, &connection.dst_len);
    connection.seq_number = 0;
    connection.client_id = rand();
}

// Sleeps the server thread for a few seconds
void RPC_idle(struct rpc_connection *rpc, int time)
{
    // first pack identifier for desired function (idle), args, client id, and seq_number into payload
    // probably use a struct to organize this
    command_req idle_call;
    
    // then send packet with this payload
    // send_packet(rpc->recv_socket, rpc->dst_addr, rpc->dst_addr, );
    
    int num_tries = 1;
    struct packet_info packet;
    while (num_tries <= 5) {
        packet = receive_packet_timeout(rpc->recv_socket, 1); // 1s timeout
        if (packet.recv_len != 0) { // valid packet
            if (0) { // TODO change this to if ACK recieved
                sleep(1);
                // send_packet()
                num_tries = 1;
                continue;
            } else if (0) { // TODO change this to if packet has other client ID or old seq number
                continue; // ignore
            } else { // correct packet recieived
                break;
            }
        } else { // socket timed out, retry
            // send_packet()
            num_tries++;
        }
    }
    if (num_tries > 5) {
        printf("Error: RPC request timed out 5 times");
        exit(1);
    }
}

// gets the value of a key on the server store
int RPC_get(struct rpc_connection *rpc, int key)
{

}

// sets the value of a key on the server store
int RPC_put(struct rpc_connection *rpc, int key, int value)
{
    
}

// closes the RPC connection to the server
void RPC_close(struct rpc_connection *rpc)
{
    close_socket(rpc->recv_socket);
}