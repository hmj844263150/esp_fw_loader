#ifndef __DOWNLOAD_API_H__
#define __DOWNLOAD_API_H__
#include "eds_config.h"

#define ESP_RAM_BLOCK    0x400

#if RUN_STUB
	#define FLASH_WRITE_SIZE 0x4000
#else 
	#define FLASH_WRITE_SIZE 0x400
#endif

#define	ESP_IMAGE_MAGIC		0xe9
#define ESP_CHECKSUM_MAGIC	0xef
#define FLASH_HEADER_OFFSET_ESP32  0x1000
#define FLASH_HEADER_OFFSET_ESP8266  0x0

//Flash sector size, minimum unit of erase.
#define FLASH_SECTOR_SIZE	0x1000

#define UART_DATA_REG_ADDR 	0x60000078

//Memory addresse
#define IROM_MAP_START	0x40200000
#define IROM_MAP_END	0x40300000

//8266 register address
#define DATE_REG_VALUE_ESP8266  0x00062000
// OTP ROM addresses
#define ESP_OTP_MAC0	0x3ff00050
#define ESP_OTP_MAC1	0x3ff00054
#define ESP_OTP_MAC3	0x3ff0005c

#define ESP8266_SPI_REG_BASE    0x60000200
#define ESP8266_SPI_W0_OFFS     0x40

//esp32 register address
#define DATE_REG_VALUE	0x15122500

#define IROM_MAP_START	0x400d0000
#define IROM_MAP_END  	0x40400000
#define DROM_MAP_START 	0x3F400000
#define DROM_MAP_END   	0x3F700000

#define ESP32_SPI_REG_BASE 		0x60002000
#define ESP32_EFUSE_REG_BASE 	0x6001a000
#define ESP32_SPI_W0_OFFS 		0x80


//ESP32 uses a 4 byte status reply
#define STATUS_BYTES_LENGTH		4
#define SPI_HAS_MOSI_DLEN_REG 	true

#define ESP_CMD_ADDR_ESP32		0x3ff42000
#define ESP_CMD_RDID_ESP32		(0x1<<28)
#define ESP_DATA_ADDR_ESP32		0x3ff42080

#define ESP_CMD_ADDR_ESP8266    0x60000200
#define ESP_CMD_RDID_ESP8266	(0x1<<28)
#define ESP_DATA_ADDR_ESP8266	0x60000240

#define ESP_CMD_ADDR_ESP8285    0x60000200
#define ESP_CMD_RDID_ESP8285	(0x1<<28)
#define ESP_DATA_ADDR_ESP8285	0x60000240

typedef enum{
	ESP8266 = 0,
	ESP32	= 1,
	ESP8285 = 2
}CHIP_TYPE;

typedef enum{
	LOAD_RAM 	= 0x1,
	LOAD_FLASH 	= 0x2,
	MDL_TEST    = 0x3,
}DOWNLOAD_MODE;

typedef enum{
	FLASH_SIZE_32_8M 	= 0x0,
	FLASH_SIZE_32_16M 	= 0x1,
	FLASH_SIZE_32_32M	= 0x2,
	FLASH_SIZE_32_64M	= 0x3,
	FLASH_SIZE_32_128M	= 0x4,
}FLASH_SIZE_ESP32;

typedef enum{
	FLASH_SIZE_8266_4M 	= 0x0,
	FLASH_SIZE_8266_2M 	= 0x1,
	FLASH_SIZE_8266_8M	= 0x2,
	FLASH_SIZE_8266_16M	= 0x3,
	FLASH_SIZE_8266_32M	= 0x4,
	FLASH_SIZE_8266_16Mb_C1= 0x5,
	FLASH_SIZE_8266_32Mb_C1= 0x6,
}FLASH_SIZE_ESP8266;

typedef enum{
	FLASH_MODE_QIO 		= 0x0,
	FLASH_MODE_QOUT		= 0x1,
	FLASH_MODE_DIO		= 0x2,
	FLASH_MODE_DOUT		= 0x3,
	FLASH_MODE_FASTRD	= 0x4,
}FLASH_MODE;

typedef enum{
	FLASH_SPEED_40M 	= 0x0,
	FLASH_SPEED_26M 	= 0x1,
	FLASH_SPEED_20M		= 0x2,
	FLASH_SPEED_80M		= 0xf,
}FLASH_SPEED;

/************************ Image data def ******************************/

typedef struct _IMAGE_HEADER{
	uint8_t magic;
	uint8_t segments;
	uint8_t flash_mode;
	uint8_t flash_size_freq;
	uint32_t entrypoint;
}IMAGE_HEADER;

#define MAX_SEGMENT_SIZE 16
typedef struct _IMAGE_DATA{
	uint32_t offset;
	uint32_t size;
}IMAGE_DATA;

typedef struct _IMAGE{
	IMAGE_HEADER header;
	IMAGE_DATA segment[MAX_SEGMENT_SIZE];
	uint8_t checksum[16]; 
}IMAGE;
/************************ end Image data def ******************************/


/************************ SD card param def ******************************/
typedef struct _LOAD_PARAM{
	uint8_t chip_type;		//ESP32, ESP8266, ESP8285
	uint8_t download_mode;	//RAM, flash
	uint8_t flash_size;		//1MB or 2MB or 4MB or 8MB or 16MB, 4MB
	uint8_t flash_speed;	//40m or 26m or 20m or 80m
	uint8_t	flash_mode;		//qio or qout or dio or dout, dio
	uint8_t bin_num; 		//no more than 8
	char image_path_divid[8][100];
	uint32_t flash_addr_divid[8];
	uint32_t port_timeout_ms;
	uint32_t flash_id;
	char chip_mac[20];
	char image_path[100];

}LOAD_PARAM;

typedef struct _VERIFY_PARAM{
	char bin_version[20];
	char tool_version[20];
	char becth_no[20];
	bool verify_en;
	char verify_value[100];
	uint8_t tool_no;		//tool NUM for factory
	uint32_t verify_tmo;
	uint32_t verify_buadrate;
}VERIFY_PARAM;

typedef struct _WIFI_PARAM{
	char wifi_ssid[65];
	char wifi_pwd[65];
}WIFI_PARAM;


typedef struct _SD_PARAM{
	LOAD_PARAM *load_param;
	WIFI_PARAM *wifi_param;
	VERIFY_PARAM *verify_param;
}SD_PARAM;
/************************ end SD card param def ******************************/


ERR_STATUS try_connect(bool esp32r0_delay);
ERR_STATUS try_load_flash(void);
ERR_STATUS try_loadParam(void);
//ERR_STATUS try_load_Firmware_Image(IMAGE *image);
ERR_STATUS try_load_ram(void);
ERR_STATUS try_write_flash(void);
void test_image(void);
ERR_STATUS _flash_begin(uint32_t usize, uint32_t uoffset);
ERR_STATUS change_baud(uint32_t baud);
ERR_STATUS read_mac(char *mac);
uint32_t read_efuse(uint32_t nth);
ERR_STATUS write_reg(uint32_t addr, uint32_t value, uint32_t mask, uint32_t delay_us);
uint32_t read_reg(uint32_t addr);
ERR_STATUS run_stub(void);
ERR_STATUS erase_flash(void);
SD_PARAM* getSdParams(void);
VERIFY_PARAM* getVerifyParams(void);
WIFI_PARAM* getWifiParams(void);
LOAD_PARAM* getLoadParams(void);
ERR_STATUS verify_flash(void);
ERR_STATUS verify_ram(void);
uint32_t get_flash_id_stub(void);
uint32_t get_flash_id(void);
ERR_STATUS get_module_result(void);
ERR_STATUS compare_result(void);
uint32_t run_spiflash_command(uint32_t spiflash_command, uint8_t* raw_data,\
								uint16_t datalen, uint32_t read_bits);

ERR_STATUS try_load_ram_from_flash(uint32_t flash_begin);


#endif //__DOWNLOAD_API_H__
