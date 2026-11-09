/* ========================= *\
 * (GII) IoT
 *
 * Práctica 2
 *
 * Gerad Lepoldo Guzmán Hernández
 * Sean Michael Owens Cruz
 *
 * Supervisor
\* ========================= */

#include <ChRt.h>
#include "../MessageProtocol.h"
#include "./Parser.h"

#define INPUT_BUFFER_SIZE 32

static THD_WORKING_AREA(wa_CommandParser, 256);
static THD_FUNCTION(commandParser, arg) {

  // Declaración de buffers para los comandos
  char inputBuffer[INPUT_BUFFER_SIZE];
  char tokens[COMMAND_TOKEN_SIZE][COMMAND_TOKEN_NUMBER];
  message msg;

  while (true) {
    // Mientras no haya input se puede saltar a otra hebra
    while (!SerialUSB.available()) {
      chThdSleepMilliseconds(50);
    };

    if (SerialUSB.available() > INPUT_BUFFER_SIZE - 1) {
      SerialUSB.println("Comando demasiado largo.");
      SerialUSB.flush();
      // Clear the input serial buffer
      while(SerialUSB.available()) {
        SerialUSB.read();
      }
      continue;
    }

    size_t bytes_read = SerialUSB.readBytes(inputBuffer, INPUT_BUFFER_SIZE - 1);
    if (bytes_read < 2) {
      SerialUSB.println("Comando demasiado corto.");
      SerialUSB.flush();
      continue;
    }
    inputBuffer[bytes_read] = '\0';

    int parse_status = tokenize(inputBuffer, tokens);
    if (parse_status != 0) {
      SerialUSB.println("Error al procesar el comando");
      continue;
    };
  }
};

void setup() {
  SerialUSB.begin(9600);
  while (!SerialUSB);

  chThdCreateStatic(wa_CommandParser, sizeof(wa_CommandParser), NORMALPRIO, commandParser, (void*)0);
}

void loop() {}
