/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"

#include "main.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
ConfigTag_t ConfigInfo =
{
		.ReaderID = {0},
		.localIPaddress = {0},
		.DHCPEnable = 1,
		.SSID = {0},
		.PWD = {0},
		.ServerHostName = {0},
		.ServerIP = {0},
};

/* UART instance and clock */
#define UART_RX_TX_IRQn UART0_RX_TX_IRQn

/* Task priorities. */
#define wifi_task_PRIORITY (configMAX_PRIORITIES - 2)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void wifi_task(void *pvParameters);

/*******************************************************************************
 * Variables
 ******************************************************************************/
struct _uart_handle t_handle;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
	uart_config_t config;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    //NVIC_SetPriority(UART_RX_TX_IRQn, 5);

    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = 9600;
    config.enableTx = true;
    config.enableRx = true;

    UART_Init(UART3, &config, CLOCK_GetFreq(UART3_CLK_SRC));

    xTaskCreate(wifi_task, "wifi_task", configMINIMAL_STACK_SIZE + 100, NULL, wifi_task_PRIORITY, NULL);

    vTaskStartScheduler();
    for (;;)
        ;
}

static void wifi_task(void *pvParameters)
{
	char result = 0;

	PRINTF("\n\rWifi_task-started\n\r");

//    strcpy (ConfigInfo.SSID, "GaraboKings");
//    strcpy (ConfigInfo.PWD, "1126300158");

    //strcpy (ConfigInfo.SSID, "Luis's iPhone");
    //strcpy (ConfigInfo.PWD, "1126300158");

//    strcpy (ConfigInfo.SSID, "RTOS");
//    strcpy (ConfigInfo.PWD, "1126300158");

	strcpy (ConfigInfo.SSID, "External-Internet");
	strcpy (ConfigInfo.PWD, "1Freescale");

    //strcpy (ConfigInfo.ServerIP, "192.168.7.2");

    //strcpy (ConfigInfo.ServerIP, "10.112.105.33");

    configWiFi();
    PRINTF("\n\rWifi_Configured\n\r");
    vTaskSuspend(NULL);


}
