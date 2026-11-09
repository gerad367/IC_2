#include "Parser.h"
COMMAND_TOKEN_NUMBER
int tokenize(char *input_buffer, char *tokens_buffer[])
{
  char *p = inpunt_buffer;
  char c;
  for (int i = 0; i < COMMAND_TOKEN_NUMBER; i++) {

    char *t = tokens_buffer[i];
    while(*p == ' ')
      p++;

    bool null_terminated = false;
    for (int j = 0; j < COMMAND_TOKEN_SIZE; j++) {
      if ((c = *p++) == ' ') {
        null_terminated = true;
        *t = '\0';
        break;
      }
      *t++ = c;
    }

    if (!null_terminated) {
      return -1;
    }
  }

  return 0;
}
