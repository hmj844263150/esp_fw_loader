#ifndef __EDS_CONFIG_H__
#define __EDS_CONFIG_H__
#include "esp_types.h"
#include "driver/uart.h"
#include "eds_error_no.h"

#define DEBUG		0
#define RUN_STUB	1
#define NEW_VERSION	1

#define USE_SPI_SEND_FW	0 //if use spi to send fw

// for wrover kit
//#define UART1_TXD  (26)
//#define UART1_RXD  (27)
//#define UART1_RTS  (18)
//#define UART1_CTS  (19)
//#define ESP_CHIP_EN	(16)
//#define ESP_GPIO_0	(17)

// for 8089 aging test board
#define UART1_TXD  (23)
#define UART1_RXD  (22)
#define ESP_CHIP_EN	(18)
#define ESP_GPIO_0	(19)
#define UART1_RTS  (18)
#define UART1_CTS  (19)


#define UART_PORT	UART_NUM_1
#define BUF_SIZE (1024)

#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22

#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BCKL 5

#define PIN_SD_MTMS	14
#define PIN_SD_MTDO	15
#define PIN_SD_D0	2
#define PIN_SD_D1	4
#define PIN_SD_MTDI	12
#define PIN_SD_MTCK	13

#define PIN_KEY		34
#define PIN_LED		32

#define WIFI_SSID	"wifi-11"
#define WIFI_PWD	"sumof1+1=2"
#define WIFI_CONN_TIMEOUT 5000 //ms

enum MODULE_STATUS{ //module opration status
	MS_DRIVER_INIT = 0,
	MS_WAIT_MODULE,
	MS_FW_DOWNLOAD,
	MS_MDL_TEST,
	MS_LOAD_RAM,
	MS_FW_CHECK,
};


#endif //__EDS_CONFIG_H__
