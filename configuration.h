#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define PRU_BASEFREQ    	40000           // PRU Base thread ISR update frequency (hz)
#define PRU_SERVOFREQ       200            // PRU Servo thread ISR update freqency (hz)

#define BASE_SLICE          0               // RP2040 PWM Slice used by the Base thread
#define SERVO_SLICE         1               // RP2040 PWM Slice used by the Servo thread

#define STEPBIT     		22            	// bit location in DDS accum
#define STEP_MASK   		(1L<<STEPBIT)

#define JSON_BUFF_SIZE	    10000			// Jason dynamic buffer size

#define JOINTS			    6				// Number of joints - set this the same as LinuxCNC HAL compenent. Max 8 joints
#define VARIABLES           2             	// Number of command values - set this the same as the LinuxCNC HAL compenent

#define PRU_DATA		    0x64617461 	    // "data" SPI payload
#define PRU_READ            0x72656164      // "read" SPI payload
#define PRU_WRITE           0x77726974      // "writ" SPI payload
#define PRU_ESTOP           0x65737470      // "estp" SPI payload
#define PRU_ACKNOWLEDGE		0x61636b6e	    // "ackn" payload
#define PRU_ERR		        0x6572726f	    // "erro" payload

#define DATA_ERR_MAX         5


// Data buffer configuration
#define BUFFER_SIZE 		68            	// Size of recieve buffer - same as HAL component, 64

#define PLL_SYS_KHZ (125 * 1000)    // 125MHz
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

#define DEFAULT_CONFIG {0x7B, 0x0A, 0x09, 0x22, 0x42, 0x6F, 0x61, 0x72, 0x64, 0x22, 0x3A, 0x20, 0x22, 0x50, 0x49, 0x43, 0x4F, 0x22, 0x2C, 0x0A, 0x09, 0x22, 0x4D, 0x6F, 0x64, 0x75, 0x6C, 0x65, 0x73, 0x22, 0x3A, 0x20, 0x5B, 0x7B, 0x0A, 0x09, 0x09, 0x22, 0x54, 0x68, 0x72, 0x65, 0x61, 0x64, 0x22, 0x3A, 0x20, 0x22, 0x53, 0x65, 0x72, 0x76, 0x6F, 0x22, 0x2C, 0x0A, 0x09, 0x09, 0x22, 0x54, 0x79, 0x70, 0x65, 0x22, 0x3A, 0x20, 0x22, 0x42, 0x6C, 0x69, 0x6E, 0x6B, 0x22, 0x2C, 0x0A, 0x09, 0x09, 0x22, 0x43, 0x6F, 0x6D, 0x6D, 0x65, 0x6E, 0x74, 0x22, 0x3A, 0x20, 0x22, 0x42, 0x6C, 0x69, 0x6E, 0x6B, 0x79, 0x22, 0x2C, 0x0A, 0x09, 0x09, 0x22, 0x50, 0x69, 0x6E, 0x22, 0x3A, 0x20, 0x22, 0x47, 0x50, 0x32, 0x35, 0x22, 0x2C, 0x0A, 0x09, 0x09, 0x22, 0x46, 0x72, 0x65, 0x71, 0x75, 0x65, 0x6E, 0x63, 0x79, 0x22, 0x3A, 0x20, 0x32, 0x0A, 0x09, 0x7D, 0x5D, 0x0A, 0x7D}

#define NUM_BLINK       1
#define NUM_STEPGEN     4

#endif