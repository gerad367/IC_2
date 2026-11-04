#ifndef __MESSAGE_PROTOCOL_H
#define __MESSAGE_PROTOCOL_H

/* OP codes
 * ======================================
 *
 * Supervisor -> Sensor
 *
 * Sensor_X OneShot
 * Sensor_X onPeriod [period : 4 bytes]
 * Sensor_X Off
 *
 * Sensor_X Config unit inc
 * Sensor_X Config unit cm
 * Sensor_X Config unit us
 *
 * Sensor_X Delay [delay : 4 bytes]
 * Sensor_X Status
 *
 * ======================================
 *
 * Sensor -> Supervisor
 *
 * Response OK
 * Response FAILURE
 * Response Status [status struct : tbd]
 * Response Measurement [measurement struct : tbd]
 *
 * ======================================
 */

typedef struct {
	char OPcode;
	char bodylen;
	char *body;
} message;

#endif
