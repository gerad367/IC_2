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
#include "./MessageProtocol.h"
#include "./Parser.h"

#define INPUT_BUFFER_SIZE 32

static THD_WORKING_AREA(wa_CommandParser, 256);
static THD_FUNCTION(commandParser, arg) {

  // Declaración de buffers para los comandos
  // char inputBuffer[INPUT_BUFFER_SIZE];
  // char tokens[COMMAND_TOKEN_SIZE][COMMAND_TOKEN_NUMBER];
  String inputBuffer;
  String tokens[COMMAND_TOKEN_NUMBER];
  message msg;

  SerialUSB.println("Comenzando loop del parser");
  while (true) {
    // Mientras no haya input se puede saltar a otra hebra
    while (!SerialUSB.available()) {
      chThdSleepMilliseconds(50);
    };

    inputBuffer = SerialUSB.readString();
    if (inputBuffer.length() < 2) {
      SerialUSB.println("Comando demasiado corto.");
      SerialUSB.flush();
      continue;
    }

    int parse_status = tokenize(inputBuffer, tokens);
    if (parse_status == -1) {
      SerialUSB.println("Error al procesar el comando");
      continue;
    };

    SerialUSB.print("Número de tokens: ");
    SerialUSB.println(parse_status);
    for (int i = 0; i < parse_status; i++) {
      SerialUSB.println(tokens[i]);
    }
    SerialUSB.flush();
  }
};

void chSetup() {
  chThdCreateStatic(wa_CommandParser, sizeof(wa_CommandParser), NORMALPRIO, commandParser, (void*)0);
}

void setup() {
  SerialUSB.begin(9600);
  while (!SerialUSB);
  SerialUSB.println("Starting");
  SerialUSB.flush();

  chBegin(chSetup);
}

void loop() {}
