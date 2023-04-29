#include "client.h"
#include <stdlib.h>

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