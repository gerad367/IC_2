#include "Parser.h"

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

// Returns the number of tokens parsed
int tokenize(String input_buffer, String tokens_buffer[])
{
  if (input_buffer.length() == 0) return -1;

  // Clean the input string
  input_buffer.strip();
  while (input_buffer.indexOf("  ") > 0) input_buffer.replace("  ", " ");

  int space_pos = input_buffer.indexOf(' ');
  if (space_pos == -1) {
    tokens_buffer[0] = input_buffer;
    return 1
  }

  int start_pos = 0;
  for (int i = 0; i < COMMAND_TOKEN_NUMBER - 1; i++) {
    tokens_buffer[i] = input_buffer.substring(start_pos, space_pos);
    start_pos = space_pos + 1;
    space_pos = input_buffer.indexOf(' ', start_pos);
    if (space_pos == -1) {
      tokens_buffer[i+1] = input_buffer.substring(start_pos);
      return i+1;
    }
  }

  tokens_buffer[3] = input_buffer.substring(start_pos, space_pos);
  return 4;
}
