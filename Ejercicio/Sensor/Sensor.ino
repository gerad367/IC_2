/* ========================= *\
 * (GII) IoT
 *
 * Práctica 2
 *
 * Gerad Lepoldo Guzmán Hernández
 * Sean Michael Owens Cruz
 *
 * Sensor
\* ========================= */

#include <Wire.h>
#include <ChRt.h>
#include "SRF02_I2C.h"
#include "./MessageProtocol.h"

#define SRF02_I2C_ADDRESS_1 byte((0xE0)>>1)
#define SRF02_I2C_ADDRESS_2 byte((0xF2)>>1)

MUTEX_DECL(Sensor1_mtx);
MUTEX_DECL(Sensor2_mtx);
MUTEX_DECL(Serial_mtx);

srf02 sensor1 = {
  &Sensor1_mtx,
  NULL,
  (char) 1,
  SRF02_I2C_ADDRESS_1,
  SRF02_UNIT_CMS,
  SRF02_STATE_PERIODIC,
  70,
  3
};

srf02 sensor2 = {
  &Sensor2_mtx,
  NULL,
  (char) 2,
  SRF02_I2C_ADDRESS_2,
  SRF02_UNIT_CMS,
  SRF02_STATE_OFF,
  70,
  0
};

static THD_WORKING_AREA(wa_Sensor1thd, 256);
static THD_WORKING_AREA(wa_Sensor2thd, 256);
static THD_WORKING_AREA(wa_Serial_thd, 256);

static THD_FUNCTION(Sensor_thread, arg)
{
  srf02* config = (srf02*) arg;
  SRF02_UNIT unit;
  SRF02_STATE state;
  char addr, id;
  short delay;
  short period;

  Serial.println("Comenzando loop en la hebra");
  while (true)
  {
    // LOAD config
    chMtxLock(config->mtx);
    addr = config->address;
    id = config->id;
    unit = config->unit;
    state = config->state;
    delay = config->delay;
    period = config->period;
    chMtxUnlock(config->mtx);

    //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    if (state == SRF02_STATE_OFF) {
      chSysLock();
      //chThdSuspendTimeoutS(&config->t_ptr, 1000);
      chThdSuspendS(&config->t_ptr);
      chSysUnlock();
      continue;
    }
    else if (state == SRF02_STATE_PULSE) {
      // Hacer y enviar medición
      //SerialUSB.println("Medición");
      measure(id, addr, unit, delay);
      chMtxLock(config->mtx);
      config->state = SRF02_STATE_OFF;
      chMtxUnlock(config->mtx);
      continue;
    }
    else if (state == SRF02_STATE_PERIODIC) {
      // Hacer y enviar medición
      //SerialUSB.println("Medición periódica");
      //SerialUSB.flush();
      measure(id, addr, unit, delay);
      chSysLock();
      chThdSuspendTimeoutS(&config->t_ptr, 1000 * period);
      chSysUnlock();
    }
  }
}

void measure(char id, char addr, SRF02_UNIT unit, short delay_val)
{
  char resp_unit;
  switch (unit) {
    case (SRF02_UNIT_INCHES):
      write_command(addr,REAL_RANGING_MODE_INCHES);
      resp_unit = CONFIG_INC;
      break;
    case (SRF02_UNIT_CMS):
      write_command(addr,REAL_RANGING_MODE_CMS);
      resp_unit = CONFIG_CM;
      break;
    case (SRF02_UNIT_USECS):
      write_command(addr,REAL_RANGING_MODE_USECS);
      resp_unit = CONFIG_US;
      break;
    default:
      return;
  }
  delay(SRF02_RANGING_DELAY);

  byte high_byte_range=read_register(addr,RANGE_HIGH_BYTE);
  byte low_byte_range=read_register(addr,RANGE_LOW_BYTE);
  byte high_min=read_register(addr,AUTOTUNE_MINIMUM_HIGH_BYTE);
  byte low_min=read_register(addr,AUTOTUNE_MINIMUM_LOW_BYTE);

  chMtxLock(&Serial_mtx);
  Serial1.write(OP_TYPE_RESPONSE | RESPONSE_MEASURE);
  Serial1.write(6);
  Serial1.write(id);
  Serial1.write(resp_unit);
  Serial1.write(high_byte_range);
  Serial1.write(low_byte_range);
  Serial1.write(high_min);
  Serial1.write(low_min);
  Serial1.flush();
  chMtxUnlock(&Serial_mtx);

  chThdSleepMilliseconds(delay_val);
}


static THD_FUNCTION(Serial_thread, arg)
{
  char op_byte, operation, action, sensor_id;
  short time_value;
  int op_len;
  char op_buff[16];
  srf02* sensor;

  while (true)
  {
    while(!Serial1.available()) {
      chThdSleepMilliseconds(50);
    }

    chMtxLock(&Serial_mtx);
    op_byte = Serial1.read();
    op_len = (int) Serial1.read();
    for (int i = 0; i < op_len; i++) {
      op_buff[i] = Serial1.read();
    }
    chMtxUnlock(&Serial_mtx);

    if (op_len == 2) {
      time_value = (short) (op_buff[1] << 8 | op_buff[0]);
    }

    if (op_byte & SELECT_SENSOR_1) {
      sensor = &sensor1;
      sensor_id = 1;
    } else if (op_byte & SELECT_SENSOR_2) {
      sensor = &sensor2;
      sensor_id = 2;
    } else {
      goto error;
    }

    chMtxLock(sensor->mtx);

    operation = op_byte & OP_MASK;
    action = op_byte & CONFIG_MASK;
    switch (operation) {
      case (OP_TYPE_OPERATION):
        switch (action) {
          case OP_OFF:
            sensor->state = SRF02_STATE_OFF;
            break;
          case OP_ONESHOT:
            sensor->state = SRF02_STATE_PULSE;
            break;
          case OP_ON_PERIOD:
            if (op_len != 2 || time_value < 1 || time_value > 1500) {
              chMtxUnlock(sensor->mtx);
              goto error;
            }
            sensor->state = SRF02_STATE_PERIODIC;
            sensor->period = time_value;
            break;
          default:
            chMtxUnlock(sensor->mtx);
            goto error;
        }
        if (sensor->t_ptr != NULL) {
          chThdResumeS(&sensor->t_ptr, (msg_t) 0);
        }
        break;
      case (OP_TYPE_CONFIG):
        switch (action) {
          case (CONFIG_INC):
            sensor->unit = SRF02_UNIT_INCHES;
            break;
          case (CONFIG_CM):
            sensor->unit = SRF02_UNIT_CMS;
            break;
          case (CONFIG_US):
            sensor->unit = SRF02_UNIT_USECS;
            break;
          case (CONFIG_DELAY):
            if (op_len != 2 || time_value < 70 || time_value > 1000) {
              chMtxUnlock(sensor->mtx);
              goto error;
            }
            sensor->delay = time_value;
            break;
          default:
            chMtxUnlock(sensor->mtx);
            goto error;
        }
        break;
      case (OP_TYPE_STATUS):
        send_status(sensor, sensor_id);
        continue;
    }
    chMtxUnlock(sensor->mtx);

    send_ok();
    continue;

error:
    send_error_feedback();
  }
}

void send_status(srf02 *sensor, char id)
{
	char mode;
	char unit;

  switch (sensor->state) {
    case (SRF02_STATE_OFF):
      mode = OP_OFF;
      break;
    case (SRF02_STATE_PERIODIC):
      mode = OP_ON_PERIOD;
      break;
  }

  switch (sensor->unit) {
    case (SRF02_UNIT_INCHES):
      unit = CONFIG_INC;
      break;
    case (SRF02_UNIT_CMS):
      unit = CONFIG_CM;
      break;
    case (SRF02_UNIT_USECS):
      unit = CONFIG_US;
      break;
  }

  chMtxLock(&Serial_mtx);
  char error = OP_TYPE_RESPONSE | RESPONSE_STATUS;
  Serial1.write(error);
  Serial1.write((char) 8);
  Serial1.write(id);
  Serial1.write(sensor->address);
  Serial1.write(mode);
  Serial1.write(unit);
  Serial1.write((char) (sensor->delay >> 8));
  Serial1.write((char) (sensor->delay & 0xff));
  Serial1.write((char) (sensor->period >> 8));
  Serial1.write((char) (sensor->period & 0xff));
  Serial1.flush();
  chMtxUnlock(&Serial_mtx);
}

void send_error_feedback()
{
  chMtxLock(&Serial_mtx);
  char error = OP_TYPE_RESPONSE | RESPONSE_FAIL;
  Serial1.write(error);
  Serial1.write((char) 0);
  Serial1.flush();
  chMtxUnlock(&Serial_mtx);
}

void send_ok()
{
  chMtxLock(&Serial_mtx);
  char error = OP_TYPE_RESPONSE | RESPONSE_OK;
  Serial1.write(error);
  Serial1.write((char) 0);
  Serial1.flush();
  chMtxUnlock(&Serial_mtx);
}

inline void write_command(byte address,byte command)
{
  Wire.beginTransmission(address);
  Wire.write(COMMAND_REGISTER); 
  Wire.write(command); 
  Wire.endTransmission();
}

byte read_register(byte address,byte the_register)
{
  Wire.beginTransmission(address);
  Wire.write(the_register);
  Wire.endTransmission();

  // getting sure the SRF02 is not busy
  Wire.requestFrom(address,byte(1));
  while(!Wire.available()) { /* do nothing */ }
  return Wire.read();
}

void chSetup() {
  chThdCreateStatic(wa_Sensor1thd, sizeof(wa_Sensor1thd), NORMALPRIO, Sensor_thread, (void*)&sensor1);
  chThdCreateStatic(wa_Sensor2thd, sizeof(wa_Sensor2thd), NORMALPRIO, Sensor_thread, (void*)&sensor2);
  chThdCreateStatic(wa_Serial_thd, sizeof(wa_Serial_thd), NORMALPRIO, Serial_thread, (void*)0);
}

void setup() {
  Serial1.begin(9600);
  //SerialUSB.begin(9600);
  Wire.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  delay(SRF02_I2C_INIT_DELAY);
  //while (!SerialUSB) {}
  //SerialUSB.println("Starting");

  chSysInit();
  chBegin(chSetup);
}

void loop() {}
