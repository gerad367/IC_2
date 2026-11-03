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
#include "SRF02_I2C.h"

#define SRF02_I2C_ADDRESS_1 byte((0xE0)>>1)
#define SRF02_I2C_ADDRESS_2 byte((0xF2)>>1)

srf02 sensor1 = {
  SRF02_I2C_ADDRESS_1,
  SRF02_UNIT_CMS,
  SRF02_STATE_OFF,
  70,
  0
};

srf02 sensor2 = {
  SRF02_I2C_ADDRESS_2,
  SRF02_UNIT_CMS,
  SRF02_STATE_OFF,
  70,
  0
};

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



void setup() {
  Wire.begin();
  delay(SRF02_I2C_INIT_DELAY);
}

void loop() {}
