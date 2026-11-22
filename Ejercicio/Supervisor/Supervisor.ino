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


MUTEX_DECL(comm_mtx);

static THD_WORKING_AREA(wa_CommandParser, 256);
static THD_FUNCTION(commandParser, arg) {

  String inputBuffer;
  String tokens[COMMAND_TOKEN_NUMBER];
  message msg;
  short period;
  short delay;
  int sensor_selected;
  char op_byte;
  char op_len;
  char op_buff[16];
  int resp_status;

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

    int token_count = tokenize(inputBuffer, tokens);
    if (token_count == -1) {
      SerialUSB.println("Error al procesar el comando.");
      SerialUSB.flush();
      continue;
    } else if (token_count == 2) {
      SerialUSB.println("Número de argumentos inválido.");
      SerialUSB.flush();
      continue;
    };

    if (tokens[0].equals(String("help"))) {
      // Imprimir información de ayuda
      print_help();
      continue;
    }

    if (!tokens[0].equals(String("us"))) {
      goto not_recognized;
    }

    if (token_count == 1) {
      // Mostrar info de los sensores
      SerialUSB.println("Sensores disponibles:");
      SerialUSB.println(" 1: Sensor 1");
      SerialUSB.println(" 2: Sensor 2");
      SerialUSB.flush();
      continue;
    }

    // Inicializamos el byte de operación a 0
    op_byte = 0;
    op_len = 0;

    sensor_selected = tokens[1].toInt();
    switch(sensor_selected) {
      case 1:
        SerialUSB.println("Sensor 1 seleccionado.");
        op_byte |= SELECT_SENSOR_1;
        break;
      case 2:
        SerialUSB.println("Sensor 2 seleccionado.");
        op_byte |= SELECT_SENSOR_2;
        break;
      default:
        SerialUSB.println("Sensor no válido.");
        continue;
    }

    if (tokens[2].equals(String("off")))
    {
      SerialUSB.println("Sensor off.");
      op_byte |= OP_OFF | OP_TYPE_OPERATION;
    }
    else if (tokens[2].equals(String("one-shot")))
    {
      SerialUSB.println("Sensor one-shot.");
      op_byte |= OP_ONESHOT | OP_TYPE_OPERATION;
    }
    else if (tokens[2].equals(String("on-period")))
    {
      if (token_count < 4) {
        SerialUSB.println("Se debe indicar el valor del periodo.");
        continue;
      }
      SerialUSB.println("Sensor on-period.");
      op_byte |= OP_ON_PERIOD | OP_TYPE_OPERATION;
      period = (short) tokens[3].toInt();
      SerialUSB.print("Valor del periodo: ");
      SerialUSB.println(period, BIN);
      if (period < 1 || period > 1500) {
        SerialUSB.println("Valor del periodo inválido.");
        continue;
      }
      op_len = sizeof(short);
      op_buff[0] = (char) (period & 0xff);
      op_buff[1] = (char) (period >> 8 & 0xff);
    }
    else if (tokens[2].equals(String("unit")))
    {
      op_byte |= OP_TYPE_CONFIG;
      if (tokens[3].equals(String("inc"))) {
        SerialUSB.println("Configurando unidad del sensor en pulgadas.");
        op_byte |= CONFIG_INC;
      } else if (tokens[3].equals(String("cm"))) {
        SerialUSB.println("Configurando unidad del sensor en centímetros.");
        op_byte |= CONFIG_CM;
      } else if (tokens[3].equals(String("us"))) {
        SerialUSB.println("Configurando unidad del sensor en microsegundos.");
        op_byte |= CONFIG_US;
      } else {
        SerialUSB.println("Unidad desconocida.");
        continue;
      }
    }
    else if (tokens[2].equals(String("delay")))
    {
      if (token_count < 4) {
        SerialUSB.println("Se debe indicar el valor del delay.");
        continue;
      }
      delay = (short) tokens[3].toInt();
      if (delay < 70 || delay > 1000) {
        SerialUSB.println("Valor del delay inválido.");
        continue;
      }
      op_byte |= OP_TYPE_CONFIG | CONFIG_DELAY;
      op_len = sizeof(short);
      op_buff[0] = (char) (delay & 0xff);
      op_buff[1] = (char) (delay >> 8 & 0xff);
    }
    else if (tokens[2].equals(String("status"))) {
      op_byte |= OP_TYPE_STATUS;
    } else {
      SerialUSB.println("Comando incorrecto o no identificado");
      continue;
    }

    SerialUSB.flush();

    chMtxLock(&comm_mtx);
    // Mandar el comando por serial1 y esperar respuesta
    Serial1.write(op_byte);
    Serial1.write(op_len);
    SerialUSB.println(op_byte, HEX);
    SerialUSB.println(op_len, HEX);
    for (int i = 0; i < op_len; i++) {
      Serial1.write(op_buff[i]);
      SerialUSB.print(op_buff[i], HEX);
    }

    SerialUSB.println("fin");
    Serial1.flush();
    //resp_status = wait_for_response();
    //is (resp_status == 1) {
    //  SerialUSB.println("Se ha recibido una respuesta de error.");
    //}
    chMtxUnlock(&comm_mtx);
    continue;

not_recognized:
    SerialUSB.println("Comando no reconocido.");
    SerialUSB.flush();
    continue;
  }
};

// 0 -> ok
// 1 -> fail
int wait_for_response()
{
  char op_byte;
  int op_len;
  char op_buff[16];

  while(!Serial1.available()) {
    chThdSleepMilliseconds(50);
  }
  op_byte = Serial1.read();
  op_len = (int) Serial1.read();
  for (int i = 0; i < op_len; i++) {
    op_buff[i] = Serial1.read();
  }

  switch (op_byte) {
    case (OP_TYPE_RESPONSE | RESPONSE_OK):
      return 0;
    case (OP_TYPE_RESPONSE | RESPONSE_FAIL):
      return 1;
    case (OP_TYPE_RESPONSE | RESPONSE_STATUS):
      print_status(op_buff);
      return 0;
    case (OP_TYPE_RESPONSE | RESPONSE_MEASURE):
      print_measurement(op_buff);
      return 0;
  }
  return 0;
}

void print_help()
{
  SerialUSB.println("Información de ayuda de los comandos:");
  SerialUSB.println("help: Muestra información acerca de los comandos aceptado.");
  SerialUSB.println("us: Muestra los sensores disponibles.");
  SerialUSB.println("us <sensor> off: Apaga el sensor.");
  SerialUSB.println("us <sensor> one-shot: Comanda una única medición.");
  SerialUSB.println("us <sensor> on-period <period>: Comanda mediciones periódicas.");
  SerialUSB.println("us <sensor> unit {inc | cm | us}: Modifica la medida devuelta por los sensores.");
  SerialUSB.println("us <sensor> delay <delay>: Programa el delay mínimo entre disparos del sensor.");
  SerialUSB.println("us <sensor> status: Muestra información del sensor.");
  SerialUSB.println();
  SerialUSB.flush();
}

static THD_WORKING_AREA(wa_MsgReceiver, 256);
static THD_FUNCTION(MsgReceiver, arg) {

  char op_byte;
  int op_len;
  char op_buff[16];
  char op_mask = OP_TYPE_RESPONSE | RESPONSE_MEASURE;
  resp_measurement resp;

  while (true) {
    chMtxLock(&comm_mtx);
    if (!Serial1.available()) {
      chMtxUnlock(&comm_mtx);
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      chThdSleepSeconds(1);
      continue;
    }

    op_byte = Serial1.read();
    op_len = (int) Serial1.read();
    if (op_byte & op_mask != op_mask || op_len != 6) {
      SerialUSB.println("Recibido un mensaje con error.");
      goto unlock;
    }
    for (int i = 0; i < op_len; i++) {
      op_buff[i] = Serial1.read();
    }

    if (op_byte == OP_TYPE_RESPONSE | RESPONSE_MEASURE) {
      print_measurement(op_buff);
    } else if (op_byte == OP_TYPE_RESPONSE | RESPONSE_STATUS) {
      print_status(op_buff);
    }
    

unlock:
    // Vaciamos el buffer de entrada por si acaso quedase información basura
    while (Serial1.available()) Serial1.read();
    chMtxUnlock(&comm_mtx);
  }
}

// input as in struct resp_status
void print_status(char* status)
{
  resp_status resp;
  resp.id = status[0];
  resp.address = status[1];
  resp.mode = status[2];
  resp.unit = status[3];

  short delay = (short) (status[4] << 8 | status[5]);
  short period = (short) (status[6] << 8 | status[7]);

  SerialUSB.println("----------------");
  SerialUSB.print("Estado del sensor ");
  SerialUSB.println(resp.id);
  SerialUSB.print("Dirección: 0x");
  SerialUSB.println(resp.address, HEX);
  SerialUSB.print("Modo: ");
  if (resp.mode == OP_OFF) {
    SerialUSB.println("OFF");
  } else if (resp.mode == OP_ON_PERIOD) {
    SerialUSB.println("ON PERIOD");
  } else {
    SerialUSB.println("unknown");
  }
  SerialUSB.print("Unidad: ");
  switch(resp.unit) {
    case (CONFIG_INC):
      SerialUSB.print(" inches");
      break;
    case (CONFIG_CM):
      SerialUSB.print(" cm");
      break;
    case (CONFIG_US):
      SerialUSB.print(" us");
      break;
    default:
      SerialUSB.print(" unknown");
      break;
  }
  SerialUSB.print("Delay: ");
  SerialUSB.println(delay);
  if (resp.mode == OP_ON_PERIOD) {
    SerialUSB.print("Periodo: ");
    SerialUSB.println(period);
  }
  SerialUSB.println("----------------");
  SerialUSB.flush();

}

// input as in struct resp_measurement
void print_measurement(char* measurement)
{
  resp_measurement resp;
  resp.id = measurement[0];
  resp.unit = measurement[1];
  resp.high_byte = measurement[2];
  resp.low_byte = measurement[3];
  resp.autotune_high_byte = measurement[4];
  resp.autotune_low_byte = measurement[5];

  short value = (short) (resp.high_byte << 8 | resp.low_byte);
  short autotune = (short) (resp.autotune_high_byte << 8 | resp.autotune_low_byte);

  SerialUSB.println("----------------");
  SerialUSB.print("Medida Sensor ");
  SerialUSB.println(resp.id);
  SerialUSB.print("Valor medido: ");
  SerialUSB.print(value);
  switch(resp.unit) {
    case (CONFIG_INC):
      SerialUSB.print(" inches");
      break;
    case (CONFIG_CM):
      SerialUSB.print(" cm");
      break;
    case (CONFIG_US):
      SerialUSB.print(" us");
      break;
    default:
      SerialUSB.print(" unknown");
      break;
  }

  SerialUSB.println();
  SerialUSB.print("Valor del autotune: ");
  SerialUSB.println(autotune);
  SerialUSB.println("----------------");
  SerialUSB.flush();
}

void chSetup()
{
  chThdCreateStatic(wa_CommandParser, sizeof(wa_CommandParser), NORMALPRIO, commandParser, (void*)0);
  chThdCreateStatic(wa_MsgReceiver, sizeof(wa_MsgReceiver), NORMALPRIO, MsgReceiver, (void*)0);

}

void setup()
{
  SerialUSB.begin(9600);
  Serial1.begin(9600);
  while (!SerialUSB);
  SerialUSB.println("Starting");
  SerialUSB.flush();
  pinMode(LED_BUILTIN, OUTPUT);

  chBegin(chSetup);
}

void loop() {}
