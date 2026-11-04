#ifndef __COMMAND_PARSER
#define __COMMAND_PARSER

#include "../MessageProtocol.h"

#define COMMAND_TOKEN_SIZE 8
#define COMMAND_TOKEN_NUMBER 4

//char tokens[COMMAND_TOKEN_SIZE][COMMAND_TOKEN_NUMBER];

int parse_command(message *msg_struct, char *input_buffer);

#endif

