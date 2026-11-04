#ifndef __MESSAGE_PROTOCOL_H
#define __MESSAGE_PROTOCOL_H

/* OP codes
 * ======================================
 *
 * Supervisor -> Sensor
 *
 * Sensor_X OneShot
 * Sensor_X onPeriod [period (s): 2 bytes]
 * Sensor_X Off
 *
 * Sensor_X Config unit inc
 * Sensor_X Config unit cm
 * Sensor_X Config unit us
 *
 * Sensor_X Delay [delay (ms): 2 bytes]
 * Sensor_X Status
 *
 * ======================================
 *
 * Sensor -> Supervisor
 *
 * Response OK ["OK" : 2 bytes]
 * Response FAILURE
 * Response Status [status struct]
 * Response Measurement [measurement struct]
 *
 * ======================================
 *
 * An OPCode of `0` shall be taken as an invalid code and
 * a FAILURE response shall be provided
 *
 * If an operation does not take any arguments and the field `bodylen` is non-zero
 * It shall be taken as an invalid operation and a FAILURE response shall be provided
 *
 * The delay of a sensor must never be less than 70ms
 * otherwise functionality is not assured
 */

typedef enum {
	Sensor_1_OneShot = 1,
	Sensor_1_onPeriod,
	Sensor_1_Off,
	Sensor_1_Config_unit_inc,
	Sensor_1_Config_unit_cm,
	Sensor_1_Config_unit_us,
	Sensor_1_Delay,
	Sensor_1_Status,
	Sensor_2_OneShot,
	Sensor_2_onPeriod,
	Sensor_2_Off,
	Sensor_2_Config_unit_inc,
	Sensor_2_Config_unit_cm,
	Sensor_2_Config_unit_us,
	Sensor_2_Delay,
	Sensor_2_Status,
	Response_OK,
	Response_FAILURE,
	Response_STATUS,
	Response_MEASUREMENT,
} OPCodes;

typedef struct {
	OPCodes OPcode;
	char bodylen;
	char *body;
} message;

typedef struct {
	char id;
	char address;
	char mode;
	char unit;
	char delay[4];
	char period[4];
} resp_status;

typedef struct {
	char id;
	char high_byte;
	char low_byte;
	char autotune_high_byte;
	char autotune_low_byte;
} resp_measurement;

#endif
