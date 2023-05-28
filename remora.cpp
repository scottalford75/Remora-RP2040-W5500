/*
Remora, RP2040 with Wiznet Ethernet, firmware for LinuxCNC
Copyright (C) 2023  Scott Alford (scotta)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 3
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>
#include <cstring>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "pico/critical_section.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/flash.h"

#include "configuration.h"
#include "remora.h"
#include "boardconfig.h"

// WIZnet
extern "C"
{
#include "wizchip_conf.h"
#include "socket.h"
#include "w5x00_spi.h"
#include "w5x00_lwip.h"
}

// Ethenet (LWIP)
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/etharp.h"
#include "tftpserver.h"

// libraries
#include "lib/ArduinoJson6/ArduinoJson.h"

// drivers
#include "drivers/pin/pin.h"

// interrupts
#include "interrupt/irqHandlers.h"
#include "interrupt/interrupt.h"

// threads
#include "thread/pruThread.h"
#include "thread/createThreads.h"

// modules
#include "modules/module.h"
#include "modules/blink/blink.h"
#include "modules/comms/RemoraComms.h"
#include "modules/debug/debug.h"
#include "modules/stepgen/stepgen.h"




/***********************************************************************
*                STRUCTURES AND GLOBAL VARIABLES                       *
************************************************************************/

// state machine
enum State {
    ST_SETUP = 0,
    ST_START,
    ST_IDLE,
    ST_RUNNING,
    ST_STOP,
    ST_RESET,
    ST_WDRESET
};

uint8_t resetCnt;
uint32_t base_freq = PRU_BASEFREQ;
uint32_t servo_freq = PRU_SERVOFREQ;

// boolean
volatile bool PRUreset;
bool configError = false;
bool threadsRunning = false;
bool staticConfig;

uint8_t noDataCount;

// pointers to objects with global scope
pruThread* servoThread;
pruThread* baseThread;
RemoraComms* comms;

// unions for RX and TX data
rxData_t rxBuffer;
volatile rxData_t rxData;
volatile txData_t txData;


// pointers to data
volatile rxData_t*  ptrRxData = &rxData;
volatile txData_t*  ptrTxData = &txData;
volatile int32_t* ptrTxHeader;  
volatile bool*    ptrPRUreset;
volatile int32_t* ptrJointFreqCmd[JOINTS];
volatile int32_t* ptrJointFeedback[JOINTS];
volatile uint8_t* ptrJointEnable;
volatile float*   ptrSetPoint[VARIABLES];
volatile float*   ptrProcessVariable[VARIABLES];
volatile uint16_t* ptrInputs;
volatile uint16_t* ptrOutputs;


// Json config file stuff
const char defaultConfig[] = DEFAULT_CONFIG;

// 512 bytes of metadata in front of actual JSON file
typedef struct
{
  uint32_t crc32;   		// crc32 of JSON
  uint32_t length;			// length in words for CRC calculation
  uint32_t jsonLength;  	// length in of JSON config in bytes
  uint8_t padding[500];
} metadata_t;
#define METADATA_LEN    512

volatile bool newJson;
uint32_t crc32;
FILE *jsonFile;
string strJson;
DynamicJsonDocument doc(JSON_BUFF_SIZE);
JsonObject thread;
JsonObject module;




static void set_clock_khz(void);
void EthernetInit();
void udpServerInit();
void EthernetTasks();
void udp_data_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);



/* Network */
extern uint8_t mac[6];
static ip_addr_t g_ip;
static ip_addr_t g_mask;
static ip_addr_t g_gateway;

/* LWIP */
struct netif g_netif;

int8_t retval = 0;
uint8_t *pack = static_cast<uint8_t *>(malloc(ETHERNET_MTU));
uint16_t pack_len = 0;
struct pbuf *p = NULL;



int8_t checkJson()
{
	metadata_t* meta = (metadata_t*)(XIP_BASE + JSON_UPLOAD_ADDRESS);
	uint32_t* json = (uint32_t*)(XIP_BASE + JSON_UPLOAD_ADDRESS + METADATA_LEN);

	// Check length is reasonable
	if (meta->length > (32/4) * FLASH_SECTOR_SIZE)
	{
		newJson = false;
		printf("JSON Config length incorrect\n");
		return -1;
	}


	// Compute CRC


	printf("Length (words) = %d\n", meta->length);
	printf("JSON length (bytes) = %d\n", meta->jsonLength);
	printf("crc32 = %x\n", crc32);



	// Check CRC
	if (crc32 != meta->crc32)
	{
		newJson = false;
		printf("JSON Config file CRC incorrect\n");
		return -1;
	}

	// JSON is OK, don't check it again
	newJson = false;
	printf("JSON Config file received Ok\n");
	return 1;
}


void jsonFromFlash(std::string json)
{
    int c;
    uint32_t i = 0;
    uint32_t jsonLength;

    printf("\n1. Loading JSON configuration file from Flash memory\n");

    // read byte 0 to determine length to read
    jsonLength = *(uint32_t*)(XIP_BASE + JSON_STORAGE_ADDRESS);

    if (jsonLength == 0xFFFFFFFF)
    {
    	printf("Flash storage location is empty - no config file\n");
    	printf("Using static configuration\n\n");

        staticConfig = true;
    }
    else
    {
		json.resize(jsonLength);

		for (i = 0; i < jsonLength; i++)
		{
			c = *(uint8_t*)(XIP_BASE + JSON_STORAGE_ADDRESS + 4 + i);
			strJson.push_back(c);
		}
		printf("\n%s\n\n", json.c_str());

        staticConfig = false;
    }
}


void deserialiseJSON()
{
    if(staticConfig) return;

    printf("\n2. Parsing JSON configuration file\n");

    const char *json = strJson.c_str();

    // parse the json configuration file
    DeserializationError error = deserializeJson(doc, json);

    printf("Config deserialisation - ");

    switch (error.code())
    {
        case DeserializationError::Ok:
            printf("Deserialization succeeded\n");
            break;
        case DeserializationError::InvalidInput:
            printf("Invalid input!\n");
            configError = true;
            break;
        case DeserializationError::NoMemory:
            printf("Not enough memory\n");
            configError = true;
            break;
        default:
            printf("Deserialization failed\n");
            configError = true;
            break;
    }

    printf("\n");
}


void configThreads()
{
    if (configError) return;

    printf("\n3. Configuring threads\n");

    JsonArray Threads = doc["Threads"];

    // create objects from JSON data
    for (JsonArray::iterator it=Threads.begin(); it!=Threads.end(); ++it)
    {
        thread = *it;

        const char* configor = thread["Thread"];
        uint32_t    freq = thread["Frequency"];

        if (!strcmp(configor,"Base"))
        {
            base_freq = freq;
            printf("Setting BASE thread frequency to %d\n", base_freq);
        }
        else if (!strcmp(configor,"Servo"))
        {
            servo_freq = freq;
            printf("Setting SERVO thread frequency to %d\n", servo_freq);
        }
    }
}


void loadStaticConfig()
{
    printf("\n4. Loading static configuration\n");

    // Servo thread modules
    
    // Ethernet communication monitoring
	comms = new RemoraComms();
	servoThread->registerModule(comms);

    loadStaticBlink();

    // Base thread modules
    loadStaticStepgen();
}


void loadModules()
{
    printf("\n4. Loading modules\n");

	// Ethernet communication monitoring
	comms = new RemoraComms();
	servoThread->registerModule(comms);

    if (configError) return;

    JsonArray Modules = doc["Modules"];

    // create objects from JSON data
    for (JsonArray::iterator it=Modules.begin(); it!=Modules.end(); ++it)
    {
        module = *it;

        const char* thread = module["Thread"];
        const char* type = module["Type"];

        if (!strcmp(thread,"Base"))
        {
            printf("\nBase thread object\n");

            if (!strcmp(type,"Stepgen"))
            {
                //createStepgen();
            }
         }
        else if (!strcmp(thread,"Servo"))
        {
        	if (!strcmp(type,"Blink"))
			{
				createBlink();
			}
        	else if (!strcmp(type,"Digital Pin"))
			{
				//createDigitalPin();
			}
        	else if (!strcmp(type,"Spindle PWM"))
			{
				//createSpindlePWM();
			}
        }
    }

}


void debugThreadHigh()
{
    printf("\n  Thread debugging.... \n\n");

    Module* debugOnB = new Debug("GP14", 1);
    baseThread->registerModule(debugOnB);

    Module* debugOnS = new Debug("GP15", 1);
    servoThread->registerModule(debugOnS);
}


void debugThreadLow()
{
    printf("\n  Thread debugging.... \n\n");

    Module* debugOffB = new Debug("GP14", 0);
    baseThread->registerModule(debugOffB);

    Module* debugOffS = new Debug("GP15", 0);
    servoThread->registerModule(debugOffS);
}


void core1_entry()
{
    enum State currentState;
    enum State prevState;

    currentState = ST_SETUP;
    prevState = ST_RESET;

    printf("\nRemora for RP2040 starting (core1)...\n\r");

    while (1)
    {
	    switch(currentState){
	        case ST_SETUP:
                // do setup tasks
                if (currentState != prevState)
                {
                    printf("\n## Entering SETUP state\n\n");
                }
                prevState = currentState;

                jsonFromFlash(strJson);
                deserialiseJSON();
                configThreads();
                createThreads();
                debugThreadHigh();
                if (staticConfig)
                {
                    loadStaticConfig();
                }
                else
                {
                    loadModules();
                }
                debugThreadLow();

                currentState = ST_START;
                break;

            case ST_START:
                // do start tasks
                if (currentState != prevState)
                {
                    printf("\n## Entering START state\n");
                }
                prevState = currentState;

                if (!threadsRunning)
                {
                    // Start the threads
                    printf("\nStarting the BASE thread\n");
                    baseThread->startThread();

                    printf("\nStarting the SERVO thread\n");
                    servoThread->startThread();

                    threadsRunning = true;
                }

                currentState = ST_IDLE;

                break;

            case ST_IDLE:
                // do something when idle
                if (currentState != prevState)
                {
                    printf("\n## Entering IDLE state\n");
                }
                prevState = currentState;

                //wait for data before changing to running state
                
                if (comms->getStatus())
                {
                    currentState = ST_RUNNING;
                }
                
                break;

            case ST_RUNNING:
                // do running tasks
                if (currentState != prevState)
                {
                    printf("\n## Entering RUNNING state\n");
                }
                prevState = currentState;
                
                if (comms->getStatus() == false)
                {
                    currentState = ST_RESET;
                }
                
                break;

            case ST_STOP:
                // do stop tasks
                if (currentState != prevState)
                {
                    printf("\n## Entering STOP state\n");
                }
                prevState = currentState;


                currentState = ST_STOP;
                break;

            case ST_RESET:
                // do reset tasks
                if (currentState != prevState)
                {
                    printf("\n## Entering RESET state\n");
                }
                prevState = currentState;

                // set all of the rxData buffer to 0
                // rxData.rxBuffer is volatile so need to do this the long way. memset cannot be used for volatile
                printf("   Resetting rxBuffer\n");
                {
                    int n = sizeof(rxData.rxBuffer);
                    while(n-- > 0)
                    {
                        rxData.rxBuffer[n] = 0;
                    }
                }

                currentState = ST_IDLE;
                break;

            case ST_WDRESET:
                // force a reset
                break;
	    }
    }

}


int main()
{
    // Network configuration
    IP4_ADDR(&g_ip, 10, 10, 10, 10);
    IP4_ADDR(&g_mask, 255, 255, 255, 0);
    IP4_ADDR(&g_gateway, 10, 10, 10, 1);

    set_clock_khz();

    // Initialize stdio after the clock change
    stdio_init_all();

    sleep_ms(1000 * 3); // wait for 3 seconds

    printf("\nRemora for RP2040 starting (core0)...\n\n\r");

    EthernetInit();
    udpServerInit();
    IAP_tftpd_init();

    // launch main Remora code on the second core
    multicore_launch_core1(core1_entry);

    while (1)
    {
        EthernetTasks();
        sys_check_timeouts();
    }
}


static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}


void EthernetInit()
{
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    // Set ethernet chip MAC address
    setSHAR(mac);
    ctlwizchip(CW_RESET_PHY, 0);

    // Initialize LWIP in NO_SYS mode
    lwip_init();

    netif_add(&g_netif, &g_ip, &g_mask, &g_gateway, NULL, netif_initialize, netif_input);
    g_netif.name[0] = 'e';
    g_netif.name[1] = '0';

    // Assign callbacks for link and status
    netif_set_link_callback(&g_netif, netif_link_callback);
    netif_set_status_callback(&g_netif, netif_status_callback);

    // MACRAW socket open
    retval = socket(SOCKET_MACRAW, Sn_MR_MACRAW, PORT_LWIPERF, 0x00);

    if (retval < 0)
    {
        printf(" MACRAW socket open failed\n");
    }

    // Set the default interface and bring it up
    netif_set_link_up(&g_netif);
    netif_set_up(&g_netif);
}


void EthernetTasks()
{
    getsockopt(SOCKET_MACRAW, SO_RECVBUF, &pack_len);

    if (pack_len > 0)
    {
        pack_len = recv_lwip(SOCKET_MACRAW, (uint8_t *)pack, pack_len);

        if (pack_len)
        {
            p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
            pbuf_take(p, pack, pack_len);
            free(pack);

            pack = static_cast<uint8_t *>(malloc(ETHERNET_MTU));
        }
        else
        {
            printf(" No packet received\n");
        }

        if (pack_len && p != NULL)
        {
            LINK_STATS_INC(link.recv);

            if (g_netif.input(p, &g_netif) != ERR_OK)
            {
                pbuf_free(p);
            }
        }
    }
}


void udpServerInit(void)
{
   struct udp_pcb *upcb;
   err_t err;

   // UDP control block for data
   upcb = udp_new();
   err = udp_bind(upcb, &g_ip, 27181);  // 27181 is the server UDP port


   /* 3. Set a receive callback for the upcb */
   if(err == ERR_OK)
   {
	   udp_recv(upcb, udp_data_callback, NULL);
   }
   else
   {
	   udp_remove(upcb);
   }
}


void udp_data_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	int txlen = 0;
	struct pbuf *txBuf;
    uint32_t status;

	// copy the UDP payload into the rxData structure
	memcpy(&rxBuffer.rxBuffer, p->payload, p->len);

	if (rxBuffer.header == PRU_READ)
	{
		txData.header = PRU_DATA;
		txlen = BUFFER_SIZE;
		comms->dataReceived();
	}
	else if (rxBuffer.header == PRU_WRITE)
	{
		txData.header = PRU_ACKNOWLEDGE;
		txlen = sizeof(txData.header);
		comms->dataReceived();

		// ensure an atomic access to the rxBuffer
        // wait for the threads not to be executing
        while (baseThread->semaphore || servoThread->semaphore){}

		// then move the data
		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			rxData.rxBuffer[i] = rxBuffer.rxBuffer[i];
		}

	}

	// allocate pbuf from RAM
	txBuf = pbuf_alloc(PBUF_TRANSPORT, txlen, PBUF_RAM);

	// copy the data into the buffer
	pbuf_take(txBuf, (char*)&txData.txBuffer, txlen);

	// Connect to the remote client
	udp_connect(upcb, addr, port);

	// Send a Reply to the Client
	udp_send(upcb, txBuf);

	// free the UDP connection, so we can accept new clients
	udp_disconnect(upcb);

	// Free the p_tx buffer
	pbuf_free(txBuf);

	// Free the p buffer
	pbuf_free(p);
}