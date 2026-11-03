#ifndef __SRF02_I2C
#define __SRF02_I2C

#define SRF02_I2C_INIT_DELAY 100 // in milliseconds
#define SRF02_RANGING_DELAY 70 // milliseconds

// LCD05's command related definitions
#define COMMAND_REGISTER byte(0x00)
#define SOFTWARE_REVISION byte(0x00)
#define RANGE_HIGH_BYTE byte(2)
#define RANGE_LOW_BYTE byte(3)
#define AUTOTUNE_MINIMUM_HIGH_BYTE byte(4)
#define AUTOTUNE_MINIMUM_LOW_BYTE byte(5)

// SRF02's command codes
#define REAL_RANGING_MODE_INCHES    byte(80)
#define REAL_RANGING_MODE_CMS       byte(81)
#define REAL_RANGING_MODE_USECS     byte(82)
#define FAKE_RANGING_MODE_INCHES    byte(86)
#define FAKE_RANGING_MODE_CMS       byte(87)
#define FAKE_RANGING_MODE_USECS     byte(88)
#define TRANSMIT_8CYCLE_40KHZ_BURST byte(92)
#define FORCE_AUTOTUNE_RESTART      byte(96)
#define ADDRESS_CHANGE_1ST_SEQUENCE byte(160)
#define ADDRESS_CHANGE_3RD_SEQUENCE byte(165)
#define ADDRESS_CHANGE_2ND_SEQUENCE byte(170)

typedef enum {
	SRF02_UNIT_INCHES,
	SRF02_UNIT_CMS,
	SRF02_UNIT_USECS
} SRF02_UNIT;

typedef enum {
	SRF02_STATE_OFF,
	SRF02_STATE_PULSE,
	SRF02_STATE_PERIODIC
} SRF02_STATE;

typedef struct {
	char address;
	SRF02_UNIT unit;
	SRF02_STATE state;
	float delay;
	float period;
} srf02;

#endif
