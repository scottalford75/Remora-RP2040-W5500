#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define PRU_BASEFREQ        100000          // PRU Base thread ISR update frequency (hz)
#define PRU_SERVOFREQ       1000            // PRU Servo thread ISR update freqency (hz)

#define BASE_SLICE          0               // RP2040 PWM Slice used by the Base thread
#define SERVO_SLICE         1               // RP2040 PWM Slice used by the Servo thread

#define STEPBIT     		22            	// bit location in DDS accum
#define STEP_MASK   		(1L<<STEPBIT)

#define JSON_BUFF_SIZE	    10000			// Jason dynamic buffer size

#define JOINTS			    6				// Number of joints - set this the same as LinuxCNC HAL compenent. Max 8 joints
#define VARIABLES           4             	// Number of command values - set this the same as the LinuxCNC HAL compenent

#define PRU_DATA		    0x64617461 	    // "data" SPI payload
#define PRU_READ            0x72656164      // "read" SPI payload
#define PRU_WRITE           0x77726974      // "writ" SPI payload
#define PRU_ESTOP           0x65737470      // "estp" SPI payload
#define PRU_ACKNOWLEDGE		0x61636b6e	    // "ackn" payload
#define PRU_ERR		        0x6572726f	    // "erro" payload

#define DATA_ERR_MAX         40


// Data buffer configuration
#define BUFFER_SIZE 		68            	// Size of recieve buffer - same as HAL component, 64

#define PLL_SYS_KHZ (125 * 1000)    // 133MHz
#define SOCKET_MACRAW 0
#define PORT_LWIPERF 5001

// For the Raspberry Pi Pico
// PICO_FLASH_SIZE_BYTES # The total size of the RP2040 flash, in bytes
// FLASH_SECTOR_SIZE     # The size of one sector, in bytes (the minimum amount you can erase) - is a value of 4kB
// FLASH_PAGE_SIZE       # The size of one page, in bytes (the mimimum amount you can write) - is a value of 256 bytes

// Use the last 64kB block of QSPI memory for persistant storage, 32kB for the upload location and the next 32kB for storage

// Location for storage of JSON config file in Flash
#define JSON_UPLOAD_ADDRESS				PICO_FLASH_SIZE_BYTES - ((2 * 32)/4) * FLASH_SECTOR_SIZE
#define JSON_STORAGE_ADDRESS 			PICO_FLASH_SIZE_BYTES - (32/4) * FLASH_SECTOR_SIZE

#include "board_list.h"
#if (DEVICE_BOARD_NAME == W55RP20_EVB_PICO)
#define DEFAULT_BLINKY_GPIO "GP19"
#else
#define DEFAULT_BLINKY_GPIO "GP25"
#endif

#define DEFAULT_CONFIG \
"{\n" \
"    \"Board\": \"PICO\",\n" \
"    \"Modules\":[\n" \
"        {\n" \
"            \"Thread\": \"Servo\",\n" \
"            \"Type\": \"Blink\",\n" \
"            \"Comment\": \"Blinky\",\n" \
"            \"Pin\": \"" DEFAULT_BLINKY_GPIO "\",\n" \
"            \"Frequency\": 2\n" \
"        },\n" \
"        {\n" \
"            \"Thread\": \"Base\",\n" \
"            \"Type\": \"Stepgen\",\n" \
"            \"Comment\": \"X - Joint 0 step generator\",\n" \
"            \"Joint Number\": 0,\n" \
"            \"Step Pin\": \"GP02\",\n" \
"            \"Direction Pin\": \"GP03\",\n" \
"            \"steplen\": 55000,\n" \
"            \"stepspace\": 55000,\n" \
"            \"dirhold\": 150000,\n" \
"            \"dirsetup\": 150000,\n" \
"            \"dirdelay\": 150000\n" \
"        },\n" \
"        {\n" \
"            \"Thread\": \"Base\",\n" \
"            \"Type\": \"Stepgen\",\n" \
"            \"Comment\": \"Y - Joint 1 step generator\",\n" \
"            \"Joint Number\": 1,\n" \
"            \"Step Pin\": \"GP04\",\n" \
"            \"Direction Pin\": \"GP05\",\n" \
"            \"steplen\": 55000,\n" \
"            \"stepspace\": 55000,\n" \
"            \"dirhold\": 150000,\n" \
"            \"dirsetup\": 150000,\n" \
"            \"dirdelay\": 150000\n" \
"        },\n" \
"        {\n" \
"            \"Thread\": \"Base\",\n" \
"            \"Type\": \"Stepgen\",\n" \
"            \"Comment\": \"Z - Joint 2 step generator\",\n" \
"            \"Joint Number\": 2,\n" \
"            \"Step Pin\": \"GP07\",\n" \
"            \"Direction Pin\": \"GP08\",\n" \
"            \"steplen\": 55000,\n" \
"            \"stepspace\": 55000,\n" \
"            \"dirhold\": 150000,\n" \
"            \"dirsetup\": 150000,\n" \
"            \"dirdelay\": 150000\n" \
"        },\n" \
"        {\n" \
"            \"Thread\": \"Base\",\n" \
"            \"Type\": \"Stepgen\",\n" \
"            \"Comment\": \"A - Joint 3 step generator\",\n" \
"            \"Joint Number\": 3,\n" \
"            \"Step Pin\": \"GP09\",\n" \
"            \"Direction Pin\": \"GP10\"\n" \
"        },\n" \
"        {\n" \
"            \"Thread\": \"Servo\",\n" \
"            \"Type\": \"Digital Pin\",\n" \
"            \"Comment\": \"X_Limit\",\n" \
"            \"Pin\": \"GP11\",\n" \
"            \"Mode\": \"Input\",\n" \
"            \"Data Bit\": 0,\n" \
"            \"Invert\": \"False\"\n" \
"        },\n" \
"        {\n" \
"            \"Thread\": \"Servo\",\n" \
"            \"Type\": \"Digital Pin\",\n" \
"            \"Comment\": \"Y_Limit\",\n" \
"            \"Pin\": \"GP12\",\n" \
"            \"Mode\": \"Input\",\n" \
"            \"Data Bit\": 1,\n" \
"            \"Invert\": \"True\"\n" \
"        },\n" \
"        {\n" \
"            \"Thread\": \"Servo\",\n" \
"            \"Type\": \"Digital Pin\",\n" \
"            \"Comment\": \"Z_Limit\",\n" \
"            \"Pin\": \"GP13\",\n" \
"            \"Mode\": \"Input\",\n" \
"            \"Data Bit\": 2,\n" \
"            \"Invert\": \"False\"\n" \
"        }\n" \
"    ]\n" \
"}"


#endif