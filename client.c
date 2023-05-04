#include "client.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "serialize_structs.h"

struct rpc_connection RPC_init(int src_port, int dst_port, char dst_addr[])
{
    struct rpc_connection connection;
    connection.recv_socket = init_socket(src_port);
    struct sockaddr_storage addr;
    socklen_t addrlen;
    populate_sockaddr(AF_INET, dst_port, dst_addr, &addr, &addrlen);
    connection.dst_addr = *((struct sockaddr *)(&addr));
    connection.dst_len = addrlen;
    connection.seq_number = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);
    connection.client_id = rand();
    return connection;
}

// Sleeps the server thread for a few seconds
void RPC_idle(struct rpc_connection *rpc, int time)
{
    // first pack client id, seq number, identifier for desired function (idle=0), and args into payload
    command idle_call;
    idle_call.client_id = rpc->client_id;
    idle_call.seq_num = rpc->seq_number++;
    idle_call.instruction_or_result = 0;
    idle_call.args[0] = time;

    char* payload = (char*)malloc(sizeof(command));
    memcpy(payload, &idle_call, sizeof(command));
    
    // then send packet with this payload
    send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
    
    int num_tries = 1;
    struct packet_info packet;
    while (num_tries <= 5) {
        packet = receive_packet_timeout(rpc->recv_socket, 1); // 1s timeout
        if (packet.recv_len != -1) { // valid packet
            command recieved;
            memcpy(&recieved, packet.buf, sizeof(command));
            if (recieved.ack) {
                sleep(1);
                send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
                num_tries = 1;
                continue;
            } else if (recieved.client_id != idle_call.client_id || recieved.seq_num < idle_call.seq_num) {
                continue; // ignore
            } else { // correct packet recieived
                break;
            }
        } else { // socket timed out, retry
            send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
            num_tries++;
        }
    }
    free(payload);
    if (num_tries > 5) {
        printf("Error: RPC request timed out 5 times\n");
        exit(1);
    }
}

// gets the value of a key on the server store
int RPC_get(struct rpc_connection *rpc, int key)
{
    // first pack client id, seq number, identifier for desired function (get=1), and args into payload
    command get_call;
    get_call.client_id = rpc->client_id;
    get_call.seq_num = rpc->seq_number++;
    get_call.instruction_or_result = 1;
    get_call.args[0] = key;

    char* payload = (char*)malloc(sizeof(command));
    memcpy(payload, &get_call, sizeof(command));
    
    // then send packet with this payload
    send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
    
    int num_tries = 1;
    struct packet_info packet;
    int get_val;
    while (num_tries <= 5) {
        packet = receive_packet_timeout(rpc->recv_socket, 1); // 1s timeout
        if (packet.recv_len != -1) { // valid packet
            command recieved;
            memcpy(&recieved, packet.buf, sizeof(command));
            if (recieved.ack) {
                sleep(1);
                send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
                num_tries = 1;
                continue;
            } else if (recieved.client_id != get_call.client_id || recieved.seq_num < get_call.seq_num) {
                continue; // ignore
            } else { // correct packet recieived
                get_val = recieved.instruction_or_result;
                break;
            }
        } else { // socket timed out, retry
            send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
            num_tries++;
        }
    }
    free(payload);
    if (num_tries > 5) {
        printf("Error: RPC request timed out 5 times\n");
        exit(1);
    }
    return get_val;
}

// sets the value of a key on the server store
int RPC_put(struct rpc_connection *rpc, int key, int value)
{
    // first pack client id, seq number, identifier for desired function (put=2), and args into payload
    command put_call;
    put_call.client_id = rpc->client_id;
    put_call.seq_num = rpc->seq_number++;
    put_call.instruction_or_result = 2;
    put_call.args[0] = key;
    put_call.args[1] = value;

    char* payload = (char*)malloc(sizeof(command));
    memcpy(payload, &put_call, sizeof(command));
    
    // then send packet with this payload
    send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
    
    int num_tries = 1;
    struct packet_info packet;
    int put_val;
    while (num_tries <= 5) {
        packet = receive_packet_timeout(rpc->recv_socket, 1); // 1s timeout
        if (packet.recv_len != -1) { // valid packet
            command recieved;
            memcpy(&recieved, packet.buf, sizeof(command));
            if (recieved.ack) {
                sleep(1);
                send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
                num_tries = 1;
                continue;
            } else if (recieved.client_id != put_call.client_id || recieved.seq_num < put_call.seq_num) {
                continue; // ignore
            } else { // correct packet recieived
                put_val = recieved.instruction_or_result;
                break;
            }
        } else { // socket timed out, retry
            send_packet(rpc->recv_socket, rpc->dst_addr, sizeof(rpc->dst_addr), payload, sizeof(command));
            num_tries++;
        }
    }
    free(payload);
    if (num_tries > 5) {
        printf("Error: RPC request timed out 5 times\n");
        exit(1);
    }
    return put_val;
}

// closes the RPC connection to the server
void RPC_close(struct rpc_connection *rpc)
{
    close_socket(rpc->recv_socket);
}