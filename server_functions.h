#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#define NUMKEYS 1024

// Sleeps the thread for a give amount of seconds seconds
void idle(int time);

// gets the value of a key on the server store
int get(int key);

// sets the value of a key on the server store
int put(int key, int value);

typedef struct command
{
    int client_id;
    int seq_num;
    int instruction;
    int args[2];
} command;

#endif