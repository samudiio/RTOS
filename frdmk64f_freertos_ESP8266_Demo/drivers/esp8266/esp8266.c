
#include "esp8266.h"
#include <stdio.h>
#include <string.h>

#include "fsl_debug_console.h"

#define printf PRINTF
//////////////////////
// Common Responses //
//////////////////////

const char R_OK[] =    "OK\r\n";
//const char R_OK[] = "READY!";
const char R_ERROR[] = "ERROR\r\n";
const char R_FAIL[] =  "FAIL";
const char R_READY1[] = "READY!";
const char R_READY2[] = "ready\r\n";

///////////////////////
// AT Commands //
///////////////////////
const char ESP_TEST[] =         "";	        // Test AT startup
const char ESP_RESET[] =        "+RST";     // Restart module
const char ESP_VERSION[] =      "+GMR";     // View version info
//const char ESP_DSLEEP[] =        "+GSLP";    // Enter deep-sleep mode
const char ESP_SLEEP[] =        "+SLEEP";    // Enter sleep mode
const char ESP_ECHO_ENABLE[] =  "E1";       // AT commands echo
const char ESP_ECHO_DISABLE[] = "E0";       // AT commands echo
//const char ESP_RESTORE[] =      "+RESTORE"; // Factory reset
const char ESP_UART[] =         "+UART_CUR";    // UART configuration for current session. Not in flash.
const char ESP_UART_DEF[] =     "+UART_DEF";    // UART configuration to be stored in flash.

////////////////////
// WiFi //
////////////////////
const char ESP_WIFI_MODE[] =    "+CWMODE";       // WiFi mode (sta/AP/sta+AP)
const char ESP_CONNECT_AP[] =   "+CWJAP";        // Connect to AP
const char ESP_LIST_AP[] =      "+CWLAP";        // List available AP's
const char ESP_DISCONNECT[] =   "+CWQAP";        // Disconnect from AP
//const char ESP_AP_CONFIG[] =    "+CWSAP";        // Set softAP configuration
const char ESP_STATION_IP[] =   "+CWLIF";        // List station IP's connected to softAP
//const char ESP_DHCP_EN[] =      "+CWDHCP";       // Enable/disable DHCP
const char ESP_AUTO_CONNECT[] = "+CWAUTOCONN";   // Connect to AP automatically
//const char ESP_SET_STA_MAC[] =  "+CIPSTAMAC";    // Set MAC address of station
//const char ESP_SET_AP_MAC[] =   "+CIPAPMAC";     // Set MAC address of softAP
const char ESP_SET_STA_IP[] =   "+CIPSTA_CUR";   // Set IP address of ESP station
//const char ESP_SET_AP_IP[] =    "+CIPAP";        // Set IP address of ESP softAP

/////////////////////
// TCP/IP //
/////////////////////
const char ESP_TCP_STATUS[] =         "+CIPSTATUS"; // Get connection status
const char ESP_TCP_CONNECT[] =        "+CIPSTART";  // Establish TCP connection or register UDP port
const char ESP_TCP_SEND[] =           "+CIPSEND";   // Send Data
const char ESP_TCP_CLOSE[] =          "+CIPCLOSE";  // Close TCP/UDP connection
const char ESP_GET_LOCAL_IP[] =       "+CIFSR";     // Get local IP address
const char ESP_TCP_MULTIPLE[] =       "+CIPMUX";    // Set multiple connections mode
const char ESP_SERVER_CONFIG[] =      "+CIPSERVER"; // Configure as server
const char ESP_TRANSMISSION_MODE[] =  "+CIPMODE";   // Set transmission mode
//const char ESP_SET_SERVER_TIMEOUT[] = "+CIPSTO";    // Set timeout when ESP runs as TCP server
const char ESP_PING[] =               "+PING";      // Function PING

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}


void sendATcmd(const char * cmd, char type, const char * params)
{
	UART_WriteBlocking(UART3, (uint8_t *)"AT", 2);
	UART_WriteBlocking(UART3, (uint8_t *)cmd, strlen(cmd));

	if (type == ESP8266_CMD_QUERY)
		UART_WriteBlocking(UART3, (uint8_t *)'?', 1);
	else if (type == ESP8266_CMD_SETUP)
	{
		UART_WriteBlocking(UART3, (uint8_t *)"=", 1);
		UART_WriteBlocking(UART3, (uint8_t *)params, strlen(params));
	}

	UART_WriteBlocking(UART3, (uint8_t *)"\r\n", 2);
}

unsigned int readByteToBuffer()
{
	uint8_t receiveBuff;
	char blockingmode = 1;
	size_t n;

	// Read the data in
	if (blockingmode)
	{
		//LPUART_DRV_ReceiveDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, &receiveBuff, 1, 1000u);
		UART_ReadBlocking(UART3, &receiveBuff, 1);
	}
	else
	{
	    // Wait to receive input data
        //LPUART_DRV_ReceiveData(WIFI_SHIELD_LPUART_INSTANCE, &receiveBuff, 1);
        //while (0x08U == LPUART_DRV_GetReceiveStatus(WIFI_SHIELD_LPUART_INSTANCE, NULL)){}
	}

	if (receiveBuff != '\0')
	{
		// Store the data in the buffer
		espRxBuffer[bufferHead] = receiveBuff;
		//if we overflow then start at the begining of the buffer
		bufferHead = (bufferHead + 1) % ESP_RX_BUFFER_LEN;
	}
	return 1;
}

void clearBuffer()
{
	memset(espRxBuffer, '\0', ESP_RX_BUFFER_LEN);
	bufferHead = 0;
}

char *searchBuffer(const char * test)
{
	int result;
	int bufferLen = strlen((const char *)espRxBuffer);
	// If our buffer isn't full, just do an strstr
	if (bufferLen <= ESP_RX_BUFFER_LEN)
	{
		result = strstr((const char *)espRxBuffer, test);
		if (result != 0x0)
		{
			return result;
		}
		return result;
	}
	else
	{
		// If the buffer is full, we need to search from the end of the
		// buffer back to the beginning.
		int testLen = strlen(test);
		for (int i=0; i<ESP_RX_BUFFER_LEN; i++)
		{

		}
	}
}

int16_t readResponse(const char * response, char timeout)
{
	unsigned int received = 0; // received keeps track of number of chars read
    int tries = 1;
    unsigned char i;

    if (timeout != 0) //If timeout is 0 then there is no timeout.
    {
    	tries = timeout*100;
    }

	clearBuffer();	// Clear the class receive buffer (espRxBuffer)

	while (tries)
	{
	    if (timeout != 0)
	    {
	    	tries -= 1;
	    }
		received += readByteToBuffer();
		if (searchBuffer(response) != 0x0)	// Search the buffer for goodRsp
			return received;	// Return how number of chars read
	}

	if (received > 0) // If we received any characters
	{
		return ESP8266_RSP_UNKNOWN; // Return unkown response error code
	}
	return ESP8266_RSP_FAIL; // Return the error code
}

int16_t readResponses(const char * responseSucess, const char * responseFailure)
{
	unsigned int received = 0; // received keeps track of number of chars read

	clearBuffer();	// Clear the class receive buffer (espRxBuffer)

	while (1)
	{
		received += readByteToBuffer();
		if (searchBuffer(responseSucess) != 0x0)	// Search the buffer for goodRsp
			return received;	// Return how number of chars read
		if (searchBuffer(responseFailure) != 0x0)	// Search the buffer for goodRsp
			return ESP8266_RSP_FAIL;	// Return how number of chars read
	}

	if (received > 0) // If we received any characters
	{
		return ESP8266_RSP_UNKNOWN; // Return unkown response error code
	}
	return ESP8266_RSP_FAIL; // Return the error code
}

char test()
{
	sendATcmd(ESP_TEST, ESP8266_CMD_EXECUTE, "");
	if (readResponse(R_OK, 0) > 0)
	{
		printf("test-Successful\n\r");
		sendATcmd(ESP_VERSION, ESP8266_CMD_EXECUTE, "");
		if (readResponse(R_OK, 0) > 0)
		{
			printf("version-Successful\n\r");
			return ESP8266_RSP_SUCCESS;
		}
		else
		{
			return ESP8266_RSP_FAIL;
		}
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

char setBaud(const char * baud)
{
	char parameters[strlen(baud) + 9];
	memset(parameters, 0, strlen(baud) + 9);

	// Put parameters into string
	sprintf(parameters, "%s,8,1,0,0", baud);

	// Send AT+UART=baud,databits,stopbits,parity,flowcontrol
	sendATcmd(ESP_UART, ESP8266_CMD_SETUP, parameters);

	if (readResponse(R_OK, 0) > 0)
	{
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

char setBaudToFlash(const char * baud)
{
	char parameters[strlen(baud) + 9];
	memset(parameters, 0, strlen(baud) + 9);

	// Put parameters into string
	sprintf(parameters, "%s,8,1,0,0", baud);

	// Send AT+UART=baud,databits,stopbits,parity,flowcontrol
	sendATcmd(ESP_UART_DEF, ESP8266_CMD_SETUP, parameters);

	if (readResponse(R_OK, 0) > 0)
	{
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

char reset()
{
	sendATcmd(ESP_RESET, ESP8266_CMD_EXECUTE, ""); // Send AT+RST

	if (readResponse(R_READY1, 200) > 0)
	{
		printf("reset-Successful\n\r");
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

int16_t setMode(char mode)
{
	char modeChar[2] = {0, 0};
	sprintf(modeChar, "%d", mode);
	sendATcmd(ESP_WIFI_MODE, ESP8266_CMD_SETUP, modeChar);

	if (readResponse(R_OK, 100) > 0)
	{
		printf("setMode-Successful\n\r");
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

int16_t getVersion(char * ATversion, char * SDKversion, char * compileTime)
{
	int16_t rsp;
	char *p, *q;

	sendATcmd(ESP_VERSION, ESP8266_CMD_EXECUTE, ""); // Send AT+GMR
	// Example Response: AT version:0.30.0.0(Jul  3 2015 19:35:49)\r\n (43 chars)
	//                   SDK version:1.2.0\r\n (19 chars)
	//                   compile time:Jul  7 2015 18:34:26\r\n (36 chars)
	//                   OK\r\n
	// (~101 characters)
	// Look for "OK":

	rsp = (readResponse(R_OK, 3) > 0);

	if (rsp > 0)
	{
		// Look for "AT version" in the rxBuffer
		p = strstr(espRxBuffer, "AT version:");
		if (p == NULL) return ESP8266_RSP_UNKNOWN;
		p += strlen("AT version:");
		q = strchr(p, '\r'); // Look for \r
		if (q == NULL) return ESP8266_RSP_UNKNOWN;
		strncpy(ATversion, p, q-p);

		// Look for "SDK version:" in the rxBuffer
		p = strstr(espRxBuffer, "SDK version:");
		if (p == NULL) return ESP8266_RSP_UNKNOWN;
		p += strlen("SDK version:");
		q = strchr(p, '\r'); // Look for \r
		if (q == NULL) return ESP8266_RSP_UNKNOWN;
		strncpy(SDKversion, p, q-p);

		// Look for "compile time:" in the rxBuffer
		p = strstr(espRxBuffer, "compile time:");
		if (p == NULL) return ESP8266_RSP_UNKNOWN;
		p += strlen("compile time:");
		q = strchr(p, '\r'); // Look for \r
		if (q == NULL) return ESP8266_RSP_UNKNOWN;
		strncpy(compileTime, p, q-p);
	}

	return rsp;
}

int16_t connect(const char * ssid, const char * pwd)
{
	int16_t rsp;

	UART_WriteBlocking(UART3, (uint8_t *)"AT", 2);
	UART_WriteBlocking(UART3, (uint8_t *)ESP_CONNECT_AP, sizeof(ESP_CONNECT_AP)-1);
	UART_WriteBlocking(UART3, (uint8_t *) "=\"", 2);
	UART_WriteBlocking(UART3, (uint8_t *) ssid, strlen(ssid));
	UART_WriteBlocking(UART3, (uint8_t *) "\"", 1);
	if (pwd != NULL)
	{
		UART_WriteBlocking(UART3, (uint8_t *) ",", 1);
		UART_WriteBlocking(UART3, (uint8_t *) "\"", 1);
		UART_WriteBlocking(UART3, (uint8_t *) pwd, strlen(pwd));
		UART_WriteBlocking(UART3, (uint8_t *) "\"", 1);
	}
	UART_WriteBlocking(UART3, (uint8_t *) "\r\n", 2);

	//rsp = readResponses(R_OK, "FAIL");
	rsp = readResponse(R_OK, 150);
	//rsp = readResponse("WIFI GOT IP", 150);
	if (rsp > 0)
	{
		printf("connect-Successful\n\r");
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

int16_t getAP(char * ssid)
{
	sendATcmd(ESP_CONNECT_AP, ESP8266_CMD_QUERY,""); // Send "AT+CWJAP?"

	int16_t rsp = readResponse(R_OK, 3);
	// Example Responses: No AP\r\n\r\nOK\r\n
	// - or -
	// +CWJAP:"WiFiSSID","00:aa:bb:cc:dd:ee",6,-45\r\n\r\nOK\r\n
	if (rsp > 0)
	{
		// Look for "No AP"
		if (strstr(espRxBuffer, "No AP") != NULL)
			return 0;

		// Look for "+CWJAP"
		char * p = strstr(espRxBuffer, ESP_CONNECT_AP);
		if (p != NULL)
		{
			p += strlen(ESP_CONNECT_AP) + 2;
			char * q = strchr(p, '"');
			if (q == NULL) return ESP8266_RSP_UNKNOWN;
			strncpy(ssid, p, q-p); // Copy string to temp char array:
			return 1;
		}
	}

	return rsp;
}

int16_t disconnect()
{
	int16_t rsp;

	sendATcmd(ESP_DISCONNECT, ESP8266_CMD_EXECUTE, ""); // Send AT+CWQAP
	// Example response: \r\n\r\nOK\r\nWIFI DISCONNECT\r\n
	// "WIFI DISCONNECT" comes up to 500ms _after_ OK.
	rsp = readResponse(R_OK, 150);
	if (rsp > 0)
	{
		printf("disconnect-Successful\n\r");
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

int16_t updateStatus()
{
	int16_t rsp;

//	sendATcmd(ESP_LIST_AP, ESP8266_CMD_EXECUTE, ""); // Send AT+CWLAP\r\n
//	rsp = readResponse(R_OK, 0);

	sendATcmd(ESP_TCP_STATUS, ESP8266_CMD_EXECUTE, ""); // Send AT+CIPSTATUS\r\n
	// Example response: (connected as client)
	// STATUS:3\r\n
	// +CIPSTATUS:0,"TCP","93.184.216.34",80,0\r\n\r\nOK\r\n
	// - or - (clients connected to ESP8266 server)
	// STATUS:3\r\n
	// +CIPSTATUS:0,"TCP","192.168.0.100",54723,1\r\n
	// +CIPSTATUS:1,"TCP","192.168.0.101",54724,1\r\n\r\nOK\r\n

	rsp = readResponse(R_OK, 50);

	{
		char * p = searchBuffer("STATUS:");
		if (p == NULL)
			return ESP8266_RSP_UNKNOWN;

		p += strlen("STATUS:");
		statusvalue.stat = (*p - 48);

		for (int i=0; i<ESP8266_MAX_SOCK_NUM; i++)
		{
			p = strstr(p, "+CIPSTATUS:");
			if (p == NULL)
			{
				// Didn't find any IPSTATUS'. Set linkID to 255.
				for (int j=i; j<ESP8266_MAX_SOCK_NUM; j++)
					statusvalue.ipstatus[j].linkID = 255;
				return rsp;
			}
			else
			{
				p += strlen("+CIPSTATUS:");
				// Find linkID:
				uint8_t linkId = *p - 48;
				if (linkId >= ESP8266_MAX_SOCK_NUM)
					return rsp;
				statusvalue.ipstatus[linkId].linkID = linkId;

				// Find type (p pointing at linkID):
				p += 3; // Move p to either "T" or "U"
				if (*p == 'T')
					statusvalue.ipstatus[linkId].type = ESP8266_TCP;
				else if (*p == 'U')
					statusvalue.ipstatus[linkId].type = ESP8266_UDP;
				else
					statusvalue.ipstatus[linkId].type = ESP8266_TYPE_UNDEFINED;

				// Find remoteIP (p pointing at first letter or type):
				p += 6; // Move p to first digit of first octet.
				for (uint8_t j = 0; j < 4; j++)
				{
					char tempOctet[4];
					memset(tempOctet, 0, 4); // Clear tempOctet

					size_t octetLength = strspn(p, "0123456789"); // Find length of numerical string:

					strncpy(tempOctet, p, octetLength); // Copy string to temp char array:
					statusvalue.ipstatus[linkId].remoteIP[j] = atoi(tempOctet); // Move the temp char into IP Address octet

					p += (octetLength + 1); // Increment p to next octet
				}

				// Find port (p pointing at ',' between IP and port:
				p += 1; // Move p to first digit of port
				char tempPort[6];
				memset(tempPort, 0, 6);
				size_t portLen = strspn(p, "0123456789"); // Find length of numerical string:
				strncpy(tempPort, p, portLen);
				statusvalue.ipstatus[linkId].port = atoi(tempPort);
				p += portLen + 1;

				// Find tetype (p pointing at tetype)
				if (*p == '0')
					statusvalue.ipstatus[linkId].tetype = ESP8266_CLIENT;
				else if (*p == '1')
					statusvalue.ipstatus[linkId].tetype = ESP8266_SERVER;
			}
		}
	}

	return rsp;
}

int16_t status()
{
	int16_t statusRet = updateStatus();
	if (statusRet > 0)
	{
		switch (statusvalue.stat)
		{
		case ESP8266_STATUS_GOTIP:        // 2 - Have IP address.
		case ESP8266_STATUS_CONNECTED:    // 3 - Connected, but haven't gotten an IP
		case ESP8266_STATUS_DISCONNECTED: // 4 - "Client" disconnected, not wifi
		case ESP8266_STATUS_NOWIFI:       // 5 - No WiFi configured
			break;
		}
	}
	return statusvalue.stat;
}

uint8_t localIP()
{
	int16_t rsp;
	char i = 0;
	char * p = 0x0;

	sendATcmd(ESP_GET_LOCAL_IP, ESP8266_CMD_EXECUTE, ""); // Send AT+CIFSR\r\n
	// Example Response: +CIFSR:STAIP,"192.168.0.114"\r\n
	//                   +CIFSR:STAMAC,"18:fe:34:9d:b7:d9"\r\n
	//                   \r\n
	//                   OK\r\n
	// Look for the OK:
	rsp = readResponse(R_OK, 3);
	if (rsp > 0)
	{
		// Look for "STAIP" in the rxBuffer
		p = strstr(espRxBuffer, "STAIP");
		if (p != NULL)
		{
			//IPAddress returnIP;

			p += 7; // Move p seven places. (skip STAIP,")

			for (i=1;i<20;i++)
			{
				p+=1;
				if (*p == 0x22) //look for the \" (0x22) character.
				{
					*p = 0x0;   //Replace the \" with the end of string 0x0
					p-=i;       //return to the begin og the IP address to then be stored
					break;
				}
			}

			//save the IP address confirmed by the WiFi module in the config structure.
			strcpy(ConfigInfo.localIPaddress, p);

			//return remoteIP;
			printf("localIP-Successful\n\r");
			return ESP8266_RSP_SUCCESS;
		}
	}

	return rsp;
}

int16_t ping(char * server)
{
	char params[strlen(server) + 3];
	int16_t rsp;

	sprintf(params, "\"%s\"", server);
	// Send AT+Ping=<server>
	sendATcmd(ESP_PING, ESP8266_CMD_SETUP, params);
	// Example responses:
	//  * Good response: +12\r\n\r\nOK\r\n
	//  * Timeout response: +timeout\r\n\r\nERROR\r\n
	//  * Error response (unreachable): ERROR\r\n\r\n
	rsp = readResponses(R_OK, R_ERROR);
	if (rsp > 0)
	{
		char * p = searchBuffer("+");
		p += 1; // Move p forward 1 space
		char * q = strchr(p, '\r'); // Find the first \r
		if (q == NULL)
			return ESP8266_RSP_UNKNOWN;
		char tempRsp[10];
		strncpy(tempRsp, p, q - p);
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		if (searchBuffer("timeout") != NULL)
		{
			return ESP8266_RSP_TIMEOUT;
		}
	}
	return rsp;
}

int16_t tcpConnect(const char * destination, const char * port, char keepaliveEnabled)
{
	int16_t rsp;

	UART_WriteBlocking(UART3, (uint8_t *) "AT", 2);
	UART_WriteBlocking(UART3, (uint8_t *) ESP_TCP_CONNECT, sizeof(ESP_TCP_CONNECT)-1);
	UART_WriteBlocking(UART3, (uint8_t *) "=\"", 2);
	UART_WriteBlocking(UART3, (uint8_t *) "TCP", 3);
	UART_WriteBlocking(UART3, (uint8_t *) "\"", 1);
	UART_WriteBlocking(UART3, (uint8_t *) ",", 1);
	UART_WriteBlocking(UART3, (uint8_t *) "\"", 1);
	UART_WriteBlocking(UART3, (uint8_t *) destination, strlen(destination));
	UART_WriteBlocking(UART3, (uint8_t *) "\"", 1);
	UART_WriteBlocking(UART3, (uint8_t *) ",", 1);
	UART_WriteBlocking(UART3, (uint8_t *) port, strlen(port));
	if (keepaliveEnabled)
	{
		UART_WriteBlocking(UART3, (uint8_t *) ",", 1);
		UART_WriteBlocking(UART3, (uint8_t *) "3500", 4);
	}

	UART_WriteBlocking(UART3, (uint8_t *) "\r\n", 2);

	rsp = readResponses(R_OK, R_ERROR);
	if (rsp > 0)
	{
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}

	return 1;
}

//int16_t setStaticIP(const char * staticIP, const char * Gateway, const char * Netmask)
//{
//	int16_t rsp;
//
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, "AT", 2, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, ESP_SET_STA_IP, sizeof(ESP_SET_STA_IP)-1, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, "=\"", 2, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, staticIP, strlen(staticIP), 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, "\"", 1, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, ",", 1, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, "\"", 1, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, Gateway, strlen(Gateway), 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, "\"", 1, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, ",", 1, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, "\"", 1, 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, Netmask, strlen(Netmask), 100);
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, "\"", 1, 100);
//
//	LPUART_DRV_SendDataBlocking(WIFI_SHIELD_LPUART_INSTANCE, "\r\n", 2, 100);
//
//	rsp = readResponse(R_OK, 0);
//	if (rsp > 0)
//	{
//		return ESP8266_RSP_SUCCESS;
//	}
//	else
//	{
//		return ESP8266_RSP_FAIL;
//	}
//}

int16_t tcpSend(const uint8_t *buf)
{
	char params[8];
	int16_t rsp;
	int16_t size = strlen(buf);

	if (size > 2048)
	{
		return ESP8266_CMD_BAD;
	}

	sprintf(params, "%d", size);
	sendATcmd(ESP_TCP_SEND, ESP8266_CMD_SETUP, params);

	rsp = readResponses(R_OK, R_ERROR);
	if (rsp != ESP8266_RSP_FAIL)
	{
		//print((const char *)buf);
		UART_WriteBlocking(UART3, (uint8_t *) buf, size);
		UART_WriteBlocking(UART3, (uint8_t *) "+IPD", 4);
		rsp = readResponse("SEND OK", 0);
		if (rsp > 0)
		{
			//Get the entire response from the server
			rsp = readResponse("200 OK", 10);
			if (rsp != ESP8266_RSP_FAIL)
			{
				return ESP8266_RSP_SUCCESS;
			}

		}
		else
		{
			return ESP8266_RSP_FAIL;
		}
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

int16_t close(uint8_t linkID)
{
	char params[2];
	int16_t rsp;

	sprintf(params, "%d", linkID);
	//sendATcmd(ESP_TCP_CLOSE, ESP8266_CMD_SETUP, params);
	sendATcmd(ESP_TCP_CLOSE, ESP8266_CMD_EXECUTE, "");

	// Eh, client virtual function doesn't have a return value.
	// We'll wait for the OK or timeout anyway.
	rsp = readResponse(R_OK, 10);
	if (rsp > 0)
	{
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

int16_t setTransferMode(uint8_t mode)
{
	char params[2] = {0, 0};
	int16_t rsp;

	params[0] = (mode > 0) ? '1' : '0';

	sendATcmd(ESP_TRANSMISSION_MODE, ESP8266_CMD_SETUP, params);

	rsp = readResponse(R_OK, 10);
	if (rsp > 0)
	{
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}

char validateServer(void)
{
	char result;

	result = test();
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		return result;
	}

	//Check if the WiFi is still up
	result = status();
	if (result == ESP8266_STATUS_NOWIFI)
	{
		ConfigInfo.Status = CONNECTING;
		result = connect(ConfigInfo.SSID, ConfigInfo.PWD);
		if (result != ESP8266_RSP_SUCCESS)
		{
			//WiFi is down
			ConfigInfo.Status = DISCONNECTED;
			return result;
		}
	}

	//try if the server host name is reachable
	result = ping(ConfigInfo.ServerHostName);
	if (result == ESP8266_RSP_SUCCESS)
	{
		//Server is up
		ConfigInfo.Status = CONNECTED;

		result = tcpConnect(ConfigInfo.ServerHostName, "80", 1);
		if (result != ESP8266_RSP_SUCCESS)
		{
			//Error must not continue
			//WiFi is down
			ConfigInfo.Status = DISCONNECTED;
			return result;
		}

	}
	else
	{
		//Server is down try the IP address
		result = ping(ConfigInfo.ServerIP);
		if (result == ESP8266_RSP_SUCCESS)
		{
			//Server is up
			ConfigInfo.Status = CONNECTED;
			//LED_WIFI_ON;

			result = tcpConnect(ConfigInfo.ServerIP, "80", 1);
			if (result != ESP8266_RSP_SUCCESS)
			{
				//Error must not continue
				//WiFi is down
				ConfigInfo.Status = DISCONNECTED;
				return result;
			}
		}
		else
		{
			ConfigInfo.Status = DISCONNECTED;

		}
	}
}

char configWiFi(void)
{
	char result;

	result = test();
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		return result;
	}

//	result = reset();
//	if (result != ESP8266_RSP_SUCCESS)
//	{
//		//Error must not continue
//		return result;
//	}

	result = setMode(1);
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		return result;
	}

	//Go into sleep mode. This should reduce to 15 mA de WiFi module power consumption
	result = wifisleep(ESP8266_PWR_MODE_MODEM);
	if (result == ESP8266_RSP_SUCCESS)
	{
		//This means that we were able to go to speep mode;
	}
	else
	{
		//This means that we were unable to go to speep mode;
	}

	result = disconnect();
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		return result;
	}

	result = status();
	if (result == ESP8266_STATUS_NOWIFI)
	{
		result = connect(ConfigInfo.SSID, ConfigInfo.PWD);
		if (result != ESP8266_RSP_SUCCESS)
		{
			//Error must not continue
			return result;
		}
	}
	else
	{
		//Error must not continue
		return result;
	}

	if (ConfigInfo.DHCPEnable == '0')
	{
		//result = setStaticIP(ConfigInfo.localIPaddress,ConfigInfo.Gateway,ConfigInfo.Netmask);
		if (result != ESP8266_RSP_SUCCESS)
		{
			//Error must not continue
			return result;
		}
	}

	result = status();
	if (result == ESP8266_STATUS_GOTIP)
	{
		result = localIP();
		if (result != ESP8266_RSP_SUCCESS)
		{
			//Error must not continue
			return result;
		}
	}
	else
	{
		//Error must not continue
		return result;
	}

//	result = ping(ConfigInfo.ServerHostName);
//	if (result == ESP8266_RSP_SUCCESS)
//	{
//		//Server is up
//		ConfigInfo.Status = CONNECTED;
//	}
//	else
//	{
//		result = ping(ConfigInfo.ServerIP);
//		if (result == ESP8266_RSP_SUCCESS)
//		{
//			//Server is up
//			ConfigInfo.Status = CONNECTED;
//		}
//		else
//		{
//			//Server is down
//			ConfigInfo.Status = DISCONNECTED;
//		}
//	}

	ConfigInfo.Status = CONNECTED;

	return result;
}

char sendDataToServer(void)
{
	char result;
	char HTTPrequest[200] = {0};
	char sen_data = 4;

	result = validateServer();
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		ConfigInfo.Status = DISCONNECTED;
		return result;
	}
	//http://192.168.7.2/datalog.php?frdm_id=myID&sensor=acc&data=0.001
	sprintf(HTTPrequest, "GET /datalog.php?frdm_id=FRMD-Profe&sensor=acc&data=%d HTTP/1.0\r\n\r\n", 1212);

	//Send http request
	result = tcpSend(HTTPrequest);
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		ConfigInfo.Status = DISCONNECTED;
		return result;
	}

	return result;
}

char sendFlipCommand(void)
{
	char result;
	char HTTPrequest[100] = {0};

	result = validateServer();
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		return result;
	}

	sprintf(HTTPrequest, "GET /flip.php?i=nxpflip1 HTTP/1.0\r\n\r\n");

	//Send http request
	result = tcpSend(HTTPrequest);
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		ConfigInfo.Status = DISCONNECTED;
	}

	return result;
}

//char sendOpenURLCommand(void)
//{
//	char result;
//	char HTTPrequest[512] = {0};
//
//	result = validateServer();
//	if (result != ESP8266_RSP_SUCCESS)
//	{
//		//Error must not continue
//		return result;
//	}
//
//	if (RegisterDemo == 0x0)
//	{
//		sprintf(HTTPrequest, "GET /openurl.php?i=nxpurl1&d=%s HTTP/1.0\r\n\r\n", DemoInfo.TagUIDstr);
//	}
//	else
//	{
//		sprintf(HTTPrequest, "GET /registerdemo.php?i=%s&u=%s&d=%s HTTP/1.0\r\n\r\n", DemoInfo.TagUIDstr, url_encode(DemoInfo.URI), url_encode(DemoInfo.Text));
//	}
//
//	//Send http request
//	result = tcpSend(HTTPrequest);
//	if (result != ESP8266_RSP_SUCCESS)
//	{
//		//Error must not continue
//		ConfigInfo.Status = DISCONNECTED;
//		return result;
//	}
//
//	return result;
//}

//char sendFollowUpCommand(void)
//{
//	char result;
//	char HTTPrequest[512] = {0};
//
//	result = validateServer();
//	if (result != ESP8266_RSP_SUCCESS)
//	{
//		//Error must not continue
//		return result;
//	}
//
//	sprintf(HTTPrequest, "GET /followup.php?i=%s.%s&v=%s HTTP/1.0\r\n\r\n", DemoInfo.TagUIDstr, CustomerInfo.TagUIDstr, url_encode(StaffInfo.vCard));
//
//	//Send http request
//	result = tcpSend(HTTPrequest);
//	if (result != ESP8266_RSP_SUCCESS)
//	{
//		//Error must not continue
//		ConfigInfo.Status = DISCONNECTED;
//		return result;
//	}
//
//	return result;
//}

char sendFollowUpCommand(void)
{
	char result;
	char HTTPrequest[512] = {0};

	result = validateServer();
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		return result;
	}

	//sprintf(HTTPrequest, "GET /followup.php?i=%s.%s&v=%s HTTP/1.0\r\n\r\n", DemoInfo.TagUIDstr, CustomerInfo.TagUIDstr, url_encode(StaffInfo.vCard));

	//Send http request
	result = tcpSend(HTTPrequest);
	if (result != ESP8266_RSP_SUCCESS)
	{
		//Error must not continue
		ConfigInfo.Status = DISCONNECTED;
		return result;
	}

	return result;
}

char wifisleep(uint8_t mode)
{
	char params[2] = {0, 0};
	int16_t rsp;

	if (mode == ESP8266_PWR_MODE_DISABLE)
	{
		params[0] =	'0';
	}
	else if (mode == ESP8266_PWR_MODE_LIGHT)
	{
		params[0] =	'1';
	}
	else if (mode == ESP8266_PWR_MODE_MODEM)
	{
		params[0] =	'2';
	}

	sendATcmd(ESP_SLEEP, ESP8266_CMD_SETUP, params);

	rsp = readResponse(R_OK, 0);
	if (rsp > 0)
	{
		printf("wifisleep-Successful\n\r");
		return ESP8266_RSP_SUCCESS;
	}
	else
	{
		return ESP8266_RSP_FAIL;
	}
}
