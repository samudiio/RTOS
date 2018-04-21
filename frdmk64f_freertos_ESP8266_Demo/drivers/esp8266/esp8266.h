#if !defined(__esp8266_H__)
#define __esp8266_H__

/*
 * ESP8266 pinout
 * 1- GND
 * 2- TXO
 * 3- GPIO2
 * 4- CHPD
 * 5- GPIO0
 * 6- RST
 * 7- RXI
 * 8- 3V
 */

///* Standard libraries includes */
//#include <stdio.h>
//#include <string.h>

/* Board files includes */
#include "board.h"
//#include "driver_config.h"
//#include <types.h>
#include <string.h>
#include "main.h"
#include "fsl_uart.h"

//Variables
extern ConfigTag_t ConfigInfo;
//extern DemoTag_t DemoInfo;
//extern CustomerTag_t CustomerInfo;
//extern StaffTag_t StaffInfo;

//Prototypes
extern char reset();
extern void sendATcmd(const char * cmd, char type, const char * params);
extern unsigned int readByteToBuffer();
extern void clearBuffer();
extern char *searchBuffer(const char * test);
extern int16_t readResponse(const char * response, char timeout);
extern int16_t readResponses(const char * responseSucess, const char * responseFailure);
extern char test();
extern int16_t setMode(char mode);
extern int16_t getVersion(char * ATversion, char * SDKversion, char * compileTime);
extern int16_t connect(const char * ssid, const char * pwd);
extern int16_t getAP(char * ssid);
extern int16_t disconnect();
extern int16_t updateStatus();
extern int16_t status();
extern uint8_t localIP();
extern int16_t ping(char * server);
extern int16_t tcpConnect(const char * destination, const char * port, char keepaliveEnabled);
extern int16_t close(uint8_t linkID);
extern char setBaud(const char * baud);
extern char setBaudToFlash(const char * baud);
extern int16_t tcpSend(const uint8_t *buf);
extern char configWiFi(void);
extern char wifisleep(uint8_t mode);
extern char sendFlipCommand(void);
extern char sendOpenURLCommand(void);
extern char sendFollowUpCommand(void);
extern char sendDataToServer(void);
extern void itoa(int n, char s[]);

//////////////////////////
// Custom GPIO //
//////////////////////////
//const char ESP_PINMODE[] =  "+PINMODE";  // Set GPIO mode (input/output)
//const char ESP_PINWRITE[] = "+PINWRITE"; // Write GPIO (high/low)
//const char ESP_PINREAD[] =  "+PINREAD";  // Read GPIO digital value

typedef enum esp_command_type {
	ESP8266_CMD_QUERY,
	ESP8266_CMD_SETUP,
	ESP8266_CMD_EXECUTE
}esp_command_type;

////////////////////////
// Buffer Definitions //
////////////////////////
#define ESP_RX_BUFFER_LEN 512 // Number of bytes in the serial receive buffer
char espRxBuffer[ESP_RX_BUFFER_LEN];
unsigned int bufferHead; // Holds position of latest byte placed in buffer.

typedef enum  {
	ESP8266_CMD_BAD = -5,
	ESP8266_RSP_MEMORY_ERR = -4,
	ESP8266_RSP_FAIL = -3,
	ESP8266_RSP_UNKNOWN = -2,
	ESP8266_RSP_TIMEOUT = -1,
	ESP8266_RSP_SUCCESS = 0
}esp8266_cmd_rsp;

typedef enum  {
	ESP8266_MODE_STA = 1,
	ESP8266_MODE_AP = 2,
	ESP8266_MODE_STAAP = 3
}esp8266_wifi_mode;

typedef enum  {
	ESP8266_PWR_MODE_DISABLE = 0,
	ESP8266_PWR_MODE_LIGHT = 1,
	ESP8266_PWR_MODE_MODEM = 2
}esp8266_wifi_sleepmode;

typedef enum  {
	ESP8266_STATUS_GOTIP = 2,
	ESP8266_STATUS_CONNECTED = 3,
	ESP8266_STATUS_DISCONNECTED = 4,
	ESP8266_STATUS_NOWIFI = 5
}esp8266_connect_status;



#define ESP8266_MAX_SOCK_NUM 5

typedef enum  {
	ESP8266_TCP,
	ESP8266_UDP,
	ESP8266_TYPE_UNDEFINED
}esp8266_connection_type;

typedef enum  {
	ESP8266_CLIENT,
	ESP8266_SERVER
}esp8266_tetype;

struct esp8266_ipstatus
{
	uint8_t linkID;
	esp8266_connection_type type;
	uint8_t remoteIP[4];  // IPv4 address //IPAddress remoteIP;
	uint16_t port;
	esp8266_tetype tetype;
}esp8266_ipstatus;

struct esp8266_status
{
	esp8266_connect_status stat;
	struct esp8266_ipstatus ipstatus[ESP8266_MAX_SOCK_NUM];
};

struct esp8266_status statusvalue;

#endif /* __esp8266_H__ */

