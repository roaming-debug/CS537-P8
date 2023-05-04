#include "server_functions.h"
#include "udp.h"
#include "serialize_structs.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct client
{
    int client_id;
    int last_seq_number;
    int last_result;
    int instruction;
    int args[2];
    int current_com_finished; // 1 indicates finished
    struct sockaddr sock;
    unsigned int slen;
    pthread_mutex_t client_mutex;
    pthread_cond_t cond_empty;
    pthread_cond_t cond_finish;
} client;

struct update_client_para
{
    int i_client_id;
    command* com;
};

struct socket s;
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

void *handle_client(void *arg)
{
    client *c = (client *)arg;
    while (1)
    {
        command response;
        if(c->instruction == 0)
        {
            idle(c->args[0]);
        }
        else if (c->instruction == 1)
        {
            response.instruction_or_result = get(c->args[0]);
        }
        else if (c->instruction == 2)
        {
            response.instruction_or_result = put(c->args[0], c->args[1]);
        }
        c->current_com_finished = 1;
        response.client_id = c->client_id;
        response.seq_num = c->last_seq_number;
        send_packet(s, c->sock, c->slen, (char*) &response, sizeof(command));
        pthread_cond_signal(&c->cond_finish);

        pthread_mutex_lock(&c->client_mutex);
        pthread_cond_wait(&c->cond_empty, &c->client_mutex);
        pthread_mutex_unlock(&c->client_mutex);
    }
    return NULL;
}

void *update_client(void *arg)
{
    struct update_client_para* para = (struct update_client_para*) arg;
    int i_client_id = para->i_client_id;
    command* com = para->com;
    if (call_table[i_client_id].last_seq_number == com->seq_num)
    {
        command response;
        response.client_id = com->client_id;
        response.seq_num = com->seq_num;
        if(call_table[i_client_id].current_com_finished == 0)
        {
            response.instruction_or_result = 1;
        }
        else
        {
            response.instruction_or_result = call_table[i_client_id].last_result;
        }
        send_packet(s, call_table[i_client_id].sock, call_table[i_client_id].slen, (char*) &response, sizeof(command));
        return NULL;
    }
    else if (call_table[i_client_id].last_seq_number > com->seq_num)
    {
        return NULL;
    }
    // command sequence number is larger than the previous command
    if (call_table[i_client_id].current_com_finished == 0)
    {
        pthread_mutex_lock(&call_table[i_client_id].client_mutex);
        pthread_cond_wait(&call_table[i_client_id].cond_finish, &call_table[i_client_id].client_mutex);
        pthread_mutex_unlock(&call_table[i_client_id].client_mutex);
    }
    call_table[i_client_id].instruction = com->instruction_or_result;
    call_table[i_client_id].args[0] = com->args[0];
    call_table[i_client_id].args[1] = com->args[1];
    call_table[i_client_id].current_com_finished = 0;
    call_table[i_client_id].last_seq_number = com->seq_num;
    call_table[i_client_id].current_com_finished = 0;
    pthread_cond_signal(&call_table[i_client_id].cond_empty);
    free(para->com);
    free(para);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("expected usage: ./server <port>\n");
        exit(0);
    }
    memset(&call_table, 0, sizeof(call_table));

    s = init_socket(atoi(argv[1]));

    pthread_t threads[CALL_TABLE_SIZE];
    pthread_t update_t[CALL_TABLE_SIZE];
    int num_update_t = 0;

    while (1)
    {
        struct packet_info packet = receive_packet(s);
        command *com = (command*) malloc(sizeof(command));
        *com = * (command *)packet.buf;
        int i_client_id;
        if ((i_client_id = index_of_client_id(com->client_id)) == -1)
        {
            client *c = (client *)malloc(sizeof(client));
            c->client_id = com->client_id;
            c->last_seq_number = -1;
            c->last_result = 0;
            c->instruction = com->instruction_or_result;
            c->args[0] = com->args[0];
            c->args[1] = com->args[1];
            c->sock = packet.sock;
            c->slen = packet.slen;
            c->current_com_finished = 0;
            pthread_mutex_init(&c->client_mutex, NULL);
            pthread_cond_init(&c->cond_empty, NULL);
            pthread_cond_init(&c->cond_finish, NULL);
            pthread_create(&threads[num_clients++], NULL, handle_client, (void *)c);
        }
        else
        {
            struct update_client_para* args = (struct update_client_para*) malloc(sizeof(struct update_client_para));
            args->i_client_id = i_client_id;
            args->com = com;
            pthread_create(&update_t[num_update_t++], NULL, update_client, (void*) args);
        }

        if(num_update_t == CALL_TABLE_SIZE)
        {
            for (size_t i = 0; i < CALL_TABLE_SIZE; i++)
            {
                pthread_join(update_t[i], NULL);
            }
            num_update_t = 0;
        }
        if (num_clients == CALL_TABLE_SIZE)
        {
            for (size_t i = 0; i < CALL_TABLE_SIZE; i++)
            {
                // will never finish
                pthread_join(threads[i], NULL);
            }
            num_clients = 0;
        }
    }

    // will never get executed
    return 0;
}
