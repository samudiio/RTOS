/*
 * main.h
 *
 */

#ifndef _MAIN_H_
#define _MAIN_H_

#include "fsl_uart.h"

/* Type definitions */
typedef enum
{
	DISCONNECTED,
	CONNECTED,
	CONNECTING
}WiFiConnectionStatus_e;

typedef struct __attribute__ ((aligned (4)))
{
	unsigned char		TagUID[7];
	char                TagUIDstr[30];
	char		        ReaderID[16];
	char	        	localIPaddress[16];
	char        		Gateway[16];
	char        		Netmask[16];
	char				DHCPEnable;
	char				SSID[30];
	char				PWD[10];
	char				ServerHostName[30];
	char				ServerIP[16];
	char       			Status;
}ConfigTag_t, *ConfigTagPtr_t;

/* Extern declarations */

/* Function prototypes */

#endif /* _MAIN_H_ */
