#ifndef __SLIP_CMD_H__
#define __SLIP_CMD_H__
#include "eds_config.h"

#define STATUS_BYTES_LENGTH 2

typedef enum{
	IF_UART = 0,
	IF_SPI = 1
}IF_TYPE;

typedef enum{
	// Commands supported by ESP8266 ROM bootloader
    ESP_FLASH_BEGIN = 0x02,
    ESP_FLASH_DATA  = 0x03,
    ESP_FLASH_END   = 0x04,
    ESP_MEM_BEGIN   = 0x05,
    ESP_MEM_END     = 0x06,
    ESP_MEM_DATA    = 0x07,
    ESP_SYNC        = 0x08,
    ESP_WRITE_REG   = 0x09,
    ESP_READ_REG    = 0x0a,

    // Some comands supported by ESP32 ROM bootloader (or -8266 w/ stub)
    ESP_SPI_SET_PARAMS = 0x0B,
    ESP_SPI_ATTACH     = 0x0D,
    ESP_CHANGE_BAUDRATE = 0x0F,
    ESP_FLASH_DEFL_BEGIN = 0x10,
    ESP_FLASH_DEFL_DATA  = 0x11,
    ESP_FLASH_DEFL_END   = 0x12,
    ESP_SPI_FLASH_MD5    = 0x13,

    // Some commands supported by stub only
    ESP_ERASE_FLASH = 0xD0,
    ESP_ERASE_REGION = 0xD1,
    ESP_READ_FLASH = 0xD2,
    ESP_RUN_USER_CODE = 0xD3
}CMD_TYPE;

typedef struct _SLIP_DATA{
	uint8_t msg_type;
	CMD_TYPE cmd;
	uint16_t data_size;
	uint32_t extra_data;
	uint8_t *data;
}SLIP_DATA;

ERR_STATUS slip_command(IF_TYPE if_type, SLIP_DATA slip_send, SLIP_DATA *slip_recv,bool wait_flag);
ERR_STATUS check_command(SLIP_DATA slip_send, SLIP_DATA *slip_recv, bool wait_flag);
void _test_image(void);


#endif //__SLIP_CMD_H__#

