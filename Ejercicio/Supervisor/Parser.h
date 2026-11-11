#ifndef __COMMAND_PARSER
#define __COMMAND_PARSER

#include "../MessageProtocol.h"

#define COMMAND_TOKEN_SIZE 8
#define COMMAND_TOKEN_NUMBER 4

int tokenize(char *input_buffer, char *tokens_buffer[]);
int tokenize(String input_buffer, String tokens_buffer[]);

#endif
