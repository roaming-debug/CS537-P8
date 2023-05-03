#ifndef SERIALIZE_STRUCTS_H
#define SERIALIZE_STRUCTS_H

typedef struct command
{
    int client_id;
    int seq_num;
    int instruction;
    int args[2];
} command;

#endif