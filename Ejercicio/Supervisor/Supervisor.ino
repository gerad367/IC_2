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

    size_t bytes_read = SerialUSB.readBytes(inputBuffer, 32);
    if (bytes_read < 2 || bytes_read == 32) {
      SerialUSB.println("Formato del comando incorrecto");
      SerialUSB.println("Un comando debe tener un mínimo de dos caracteres y un máximo de 32");
      continue;
    }

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
