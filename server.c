#include "server_functions.h"
#include "udp.h"
#include "serialize_structs.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct client {
    int client_id;
    int last_seq_number;
    int last_result;
    pthread_mutex_t client_mutex;
};

#define CALL_TABLE_SIZE 100
struct client call_table[CALL_TABLE_SIZE];
int num_clients = 0;
pthread_mutex_t num_clients_mutex;

// find the index of the entry which has the same client_id as specified by the parameter
// return -1 if failed to find the entry
int index_of_client_id(int client_id)
{
    for (size_t i = 0; i < CALL_TABLE_SIZE; i++)
    {
        if (call_table[i].client_id == client_id)
        {
            return i;
        }
    }
    return -1;
}


void* handle_packet(void* arg)
{
    struct packet_info* packet = (struct packet_info*) arg;
    command* com = (command *) packet->buf;
    if(index_of_client_id(com->client_id) == -1)
    {
        if (num_clients >= CALL_TABLE_SIZE)
        {
            // the number of clients exceeds the constraint
            return NULL;
        }
        pthread_mutex_lock(&num_clients_mutex);
        num_clients++;
        pthread_mutex_unlock(&num_clients_mutex);
    }
    free(arg);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("expected usage: ./server <port>\n");
        exit(0);
    }
    struct socket s = init_socket(atoi(argv[1]));

    pthread_t threads[20];
    const int thread_size = 20;
    int threads_used = 0;

    while (1) {
        struct packet_info* packet = (struct packet_info*) malloc(sizeof(struct packet_info));
        *packet = receive_packet(s);
        pthread_create(&threads[threads_used++], NULL, handle_packet, (void*) packet);
        if (threads_used == thread_size)
        {
            for (size_t i = 0; i < thread_size; i++)
            {
                pthread_join(threads[i], NULL);
            }
        }
    }

    // will never get executed
    return 0;
}