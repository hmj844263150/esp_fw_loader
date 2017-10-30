#include "_download_api.h"
#include "_slip_cmd.h"
#include "_sd_api.h"

#include "esp_log.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "driver/gpio.h"
#include <math.h>

#include <sys/unistd.h>
#include "esp_vfs_fat.h"
#include "eds_driver_init.h"

#include "soc/soc.h"
#include "soc/rtc.h"

#include "lcd_api.h"
#include "lcd_func.h"




#define PULL_HIGH_IO0	gpio_set_level(ESP_GPIO_0, 1);
#define PULL_LOW_IO0	gpio_set_level(ESP_GPIO_0, 0);
#define ENABLE_CHIP		gpio_set_level(ESP_CHIP_EN, 1);
#define RESET_CHIP		gpio_set_level(ESP_CHIP_EN, 0);

#define MAC_ADDR_FORMAT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDR(x)	*(x),*(x+1),*(x+2),*(x+3),*(x+4),*(x+5)


static const char* TAG = "_download api";


//LOAD_PARAM sys_load_param = {
static LOAD_PARAM _load_param = {
	.download_mode = LOAD_FLASH,	
	.flash_size = FLASH_SIZE_8266_32M, 	
	.flash_speed = FLASH_SPEED_40M,	
	.flash_mode = FLASH_MODE_DIO, 	
	.chip_type = ESP8266,	
	.image_path = "",
	.port_timeout_ms = 10,
	.chip_mac = "",
	.bin_num = 0,
};

static VERIFY_PARAM _verify_param = {
	.verify_en = false,
	.verify_value = "",
	.verify_tmo = 0,
	.verify_buadrate = 115200,
	.tool_no = 1,
};

static WIFI_PARAM _wifi_param = {
	.wifi_ssid = "hehe",
	.wifi_pwd = "12345678",
};

static SD_PARAM g_sd_parameters = {
	.load_param = &_load_param,
	.wifi_param = &_wifi_param,
	.verify_param = &_verify_param,
};

LOAD_PARAM* getLoadParams(void){
	return &_load_param;
}


WIFI_PARAM* getWifiParams(void){
	return &_wifi_param;
}

VERIFY_PARAM* getVerifyParams(void){
	return &_verify_param;
}


SD_PARAM* getSdParams(void){
	return &g_sd_parameters;
}


static ERR_STATUS _chip_sync(void){
	uint32_t old_tmo = _load_param.port_timeout_ms;
	_load_param.port_timeout_ms = 10;
	
	SLIP_DATA sync_data, recv_data;
	uint8_t sendData[36], recvData[50];
	sync_data.data = sendData;
	recv_data.data = recvData;
	
	sync_data.msg_type = 0x00;
	sync_data.cmd = ESP_SYNC;
	sync_data.data_size = 36;
	sync_data.extra_data = 0x0;
	*(uint32_t*)(sync_data.data) = 0x20120707;
	memset(sync_data.data+4, 0x55, 32);	

	ERR_STATUS errno = 0;
	
	errno = slip_command(IF_UART, sync_data, &recv_data, true);
	//printf("%s, %d\n", __func__, __LINE__);

	_load_param.port_timeout_ms = old_tmo;
	if(errno == SUCCESS)
		return SUCCESS;
	else
		return ERR_FW_DOWNLOAD;
}

ERR_STATUS try_connect(bool esp32r0_delay){
	if(DEBUG) printf("Connecting...\n");
	uart_flush(UART_PORT);
	PULL_HIGH_IO0;
	RESET_CHIP;
	vTaskDelay(100 / portTICK_RATE_MS);
	if(esp32r0_delay) vTaskDelay(1200 / portTICK_RATE_MS);
	PULL_LOW_IO0;
	ENABLE_CHIP;
	if(esp32r0_delay) vTaskDelay(400 / portTICK_RATE_MS);
	vTaskDelay(50 / portTICK_RATE_MS);
	PULL_HIGH_IO0;

	//try 10 times to sync baud rate
	for(int i=0; i<10; i++){
		uart_flush(UART_PORT);
		if(_chip_sync() == SUCCESS) return SUCCESS;
	}
	return ERR_FW_DOWNLOAD;
}

ERR_STATUS verify_flash(void){
	RESET_CHIP;
	PULL_HIGH_IO0;
	vTaskDelay(50 / portTICK_RATE_MS);
	ENABLE_CHIP;
	VERIFY_PARAM *verify_param = getVerifyParams();
	uart_set_baudrate(UART_PORT, verify_param->verify_buadrate);
	uint8_t recvDate[BUF_SIZE];
	uint32_t len=0;
	for(int i=0; i<verify_param->verify_tmo*100; i++){
		len = uart_read_bytes(UART_PORT, recvDate, BUF_SIZE, 10 / portTICK_RATE_MS);
		printf("%s\n", recvDate);
		if(strstr((char*)recvDate, (char*)verify_param->verify_value) != NULL)
			return SUCCESS;
	}
	return 1;
}

ERR_STATUS verify_ram(void){
	VERIFY_PARAM *verify_param = getVerifyParams();
	ENABLE_CHIP;
	uart_set_baudrate(UART_PORT, verify_param->verify_buadrate);
	uint8_t recvData[BUF_SIZE];
	uint32_t len=0;
	for(int i=0; i<verify_param->verify_tmo*10; i++){
		len = uart_read_bytes(UART_PORT, recvData, BUF_SIZE, 100 / portTICK_RATE_MS);
		if(len > 0){
			printf("%s\n", recvData);
#if DEBUG
			//print recv raw data
			printf("%d:len:%d\n",i,len);
			for(int j=0; j<len; j++)
				printf("%02x ", *(recvData+j));
			printf("\n");
#endif
		}
		if(strstr((char*)recvData, (char*)verify_param->verify_value) != NULL)
			return SUCCESS;
	}
	return 1;
}


ERR_STATUS try_load_flash(void){
	ERR_STATUS errno = 0;
	
	return SUCCESS;
}

ERR_STATUS try_get_Param(void){
	//need read param from SD card here
	return SUCCESS;
}

ERR_STATUS try_get_Firmware_Image(char *image_path, IMAGE *image){
	if(get_Image(image_path, image, _load_param.chip_type) != SUCCESS){
		if(DEBUG) printf("get image failed\n");
		return ERR_FW_DOWNLOAD;
	}
	return SUCCESS;
}


//stub only
ERR_STATUS erase_flash(void){
	uint32_t old_tmo = _load_param.port_timeout_ms;

    _load_param.port_timeout_ms = 500;

	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	uint8_t sendData[16], recvData[50];
	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_ERASE_FLASH;
	send_data.data_size = 0;
	send_data.extra_data = 0x0;
	
    if(check_command(send_data, &recv_data ,true) != 0){
		if(DEBUG) printf("err:erase_flash failed\n");
		return 1;
	}
    _load_param.port_timeout_ms = old_tmo;
    return SUCCESS;
}


static ERR_STATUS _mem_begin(int size, int blocks, int blocksize, uint32_t offset){
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	uint8_t sendData[16], recvData[50];
	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_MEM_BEGIN;
	send_data.data_size = 16;
	send_data.extra_data = 0x0;
	*(int*)(send_data.data) = size;
	*(int*)(send_data.data+4) = blocks;
	*(int*)(send_data.data+8) = blocksize;
	*(int*)(send_data.data+12) = offset;
	/*
	for(int i=0; i<16; i++)
		if(DEBUG) printf("%02x ", *(send_data.data+i));
	if(DEBUG) printf("\n");
	*/
	if(check_command(send_data, &recv_data ,true) != 0){
		if(DEBUG) printf("err:send start failed\n");
		return 1;
	}
	return SUCCESS;
}

static uint32_t _checksum(uint8_t *data, uint32_t len){
	uint32_t checksum = ESP_CHECKSUM_MAGIC;
	for(int i=0; i<len; i++)
		checksum ^= *(data+i);
	return checksum;
}

static ERR_STATUS _mem_block(uint8_t *data, uint32_t seq, int datalen){
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	int len = datalen+16;
	uint8_t sendData[ESP_RAM_BLOCK+16], recvData[ESP_RAM_BLOCK];
	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_MEM_DATA;
	send_data.data_size = len;
	send_data.extra_data = _checksum(data, datalen);
	*(uint32_t*)send_data.data = datalen;
	*(uint32_t*)(send_data.data+4) = seq;
	memset(send_data.data+8, 0, 8);
	memcpy(send_data.data+16, data, datalen);
	
	if(check_command(send_data, &recv_data, true) != 0){
		return 1;
	}
	return SUCCESS;
}

static ERR_STATUS _mem_finish(uint32_t entrypoint){
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	uint8_t sendData[8], recvData[50];
	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_MEM_END;
	send_data.data_size = 8;
	send_data.extra_data = 0x0;
	if(entrypoint == 0)
		*(int*)(send_data.data) = 1;
	else
		*(int*)(send_data.data) = 0;
	*(uint32_t*)(send_data.data+4) = entrypoint;
	/*
	for(int i=0; i<16; i++)
		if(DEBUG) printf("%02x ", *(send_data.data+i));
	if(DEBUG) printf("\n");
	*/
	if(check_command(send_data, &recv_data, false) != 0){
		if(DEBUG) printf("err:send finish failed\n");
		return 1;
	}
	return SUCCESS;
}


ERR_STATUS try_load_ram(void){
	ERR_STATUS errno = 0;
	SLIP_DATA slip_send={
				.msg_type 	= 0,
				.cmd 		= 0,
				.data_size	= 0,
				.extra_data	= 0,
				};
	
	IMAGE image = {.segment = NULL};
	
	try_get_Firmware_Image(_load_param.image_path, &image);
	
	uint32_t offset, foffset, datalen;
	int size;
	int16_t seq=0;
	uint8_t data[ESP_RAM_BLOCK];
	IMAGE_DATA itSegment;

	for(int i=0; i<image.header.segments; i++){
		ESP_LOGI(TAG, "segment%d:  offset:%08x, size:%08x\n", i, image.segment[i].offset, image.segment[i].size);
	}

	//data = (uint8_t*)malloc(ESP_RAM_BLOCK);

	FILE* f = fopen(_load_param.image_path, "rb");
    if (f == NULL) {
        ESP_LOGW(TAG, "Failed to open file for reading:%s", _load_param.image_path);
		fclose(f);
        return 3;
    }
	if(_load_param.chip_type == ESP32)
		fseek(f, 32, SEEK_SET);
	else
		fseek(f, 16, SEEK_SET); 
	
	for(int i=0; i<image.header.segments; i++){
		offset = image.segment[i].offset;
		size = image.segment[i].size;

		if(DEBUG) printf("now send segment%d, offset:%04x, size:%04x\n", i, offset, size);
		if(_mem_begin(size, ceil(1.0*size/ESP_RAM_BLOCK), ESP_RAM_BLOCK, offset) != 0){
			if(DEBUG) printf("send mem begin error\n");
			fclose(f);
			return 1;
		}
		
		seq=0;
		
		while(size > 0){
			datalen = size < ESP_RAM_BLOCK ? size : ESP_RAM_BLOCK;
			//get_SegmentData(stub_path, data, foffset+(seq*ESP_RAM_BLOCK), datalen);
			fread(data, datalen, 1, f);
			if(_mem_block(data, seq, datalen)==0){
#if DEBUG
				if(DEBUG) printf("send block %d success; ", seq);
				if(DEBUG) printf("size:%d\n", size);
#endif
			}
			else{
				if(DEBUG) printf("send block %d failed\n", seq);
				fclose(f);
				return 2;
			}
			size -= ESP_RAM_BLOCK;
			seq ++;
		}
		
		fseek(f, 8, SEEK_CUR);
	}
	_mem_finish(image.header.entrypoint);
	//if(data != NULL)
	//	free(data);
	fclose(f);
	return SUCCESS;
}

static ERR_STATUS _flash_spi_attach(bool b_isHspi, bool b_isLegacy){
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	uint8_t sendData[8], recvData[50];
	send_data.data = sendData;
	recv_data.data = recvData;
	
	memset(sendData, 0, 8);
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_SPI_ATTACH;
	send_data.data_size = 8;
	send_data.extra_data = 0x0;
	
	if(b_isHspi)
		*(int*)(send_data.data) = 0x00000001;
	if(b_isLegacy)
		*(send_data.data+4) = 0x01;
	
	/*
	for(int i=0; i<16; i++)
		if(DEBUG) printf("%02x ", *(send_data.data+i));
	if(DEBUG) printf("\n");
	*/
	if(check_command(send_data, &recv_data ,true) != 0){
		if(DEBUG) printf("err:flash_spi_attach failed\n");
		return 1;
	}
	return SUCCESS;
}

static int _get_erase_size(uint32_t offset, uint32_t size){
	if(_load_param.chip_type == ESP32)
		return size;
	
	int sectors_per_block = 16;
    int sector_size = FLASH_SECTOR_SIZE; //4KB
    int num_sectors = (size + sector_size - 1) / sector_size;
    int start_sector = offset / sector_size;

    uint32_t head_sectors = sectors_per_block - (start_sector % sectors_per_block);
    if(num_sectors < head_sectors)
        head_sectors = num_sectors;

    if(num_sectors < 2 * head_sectors)
        return (num_sectors + 1) / 2 * sector_size;
    else
        return (num_sectors - head_sectors) * sector_size;
}

ERR_STATUS _flash_begin(uint32_t usize, uint32_t uoffset){
	uint32_t old_tmo = _load_param.port_timeout_ms;
    uint32_t num_blocks = (usize + FLASH_WRITE_SIZE - 1) / FLASH_WRITE_SIZE ;
    uint32_t erase_size = usize;//(usize + FLASH_WRITE_SIZE - 1);//_get_erase_size(uoffset, usize);

    _load_param.port_timeout_ms = 500;

	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	uint8_t sendData[16], recvData[50];
	send_data.data = sendData;
	recv_data.data = recvData;
	
	//memset(sendData, 0, 16);
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_FLASH_BEGIN;
	send_data.data_size = 16;
	send_data.extra_data = 0x0;

	*(uint32_t*)sendData = erase_size;
	*(uint32_t*)(sendData+4) = num_blocks;
	*(uint32_t*)(sendData+8) = FLASH_WRITE_SIZE;
	*(uint32_t*)(sendData+12) = uoffset;
	
    if(check_command(send_data, &recv_data ,true) != 0){
		if(DEBUG) printf("err:_flash_begin failed\n");
		return 1;
	}
	
	//if size != 0 and not self.IS_STUB:
    //    print("Took %.2fs to erase flash block" % (time.time() - t))
    _load_param.port_timeout_ms = old_tmo;
    return SUCCESS;
}

static ERR_STATUS _flash_block(uint8_t *data, uint32_t seq, uint32_t datalen){
	uint32_t old_tmo = _load_param.port_timeout_ms;
	_load_param.port_timeout_ms = 50; //10ms wait for response
	
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	int len = datalen+16;
	uint8_t sendData[FLASH_WRITE_SIZE+16], recvData[FLASH_WRITE_SIZE];
	/*
	if(seq == 0){
		data[2] = 0x02;
		data[3] = 0x20;
	}
	*/
	//maybe need to repalce the head of image; the combine's don't need tihs
	//replaceImage(); 
	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_FLASH_DATA;
	send_data.data_size = len;
	send_data.extra_data = _checksum(data, FLASH_WRITE_SIZE);
	*(uint32_t*)send_data.data = datalen;
	*(uint32_t*)(send_data.data+4) = seq;
	memset(send_data.data+8, 0, 8);
	memcpy(send_data.data+16, data, datalen);

	//printf("checksum:%d\n", send_data.extra_data);
	_load_param.port_timeout_ms = old_tmo;
	if(check_command(send_data, &recv_data, true) != 0){
		return 1;
	}
	return SUCCESS;
}

static ERR_STATUS _flash_finish(uint32_t reboot){
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	uint8_t sendData[8], recvData[50];
	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_FLASH_END;
	send_data.data_size = 8;
	send_data.extra_data = 0x0;
	if(reboot == 0)
		*(int*)(send_data.data) = 1;
	else
		*(int*)(send_data.data) = 0;

	if(check_command(send_data, &recv_data, true) != 0){
		if(DEBUG) printf("err:send finish failed\n");
		return 1;
	}
	return SUCCESS;
}

ERR_STATUS change_baud(uint32_t baud){
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};
	uint8_t sendData[8], recvData[50];
	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_CHANGE_BAUDRATE;
	send_data.data_size = 8;
	send_data.extra_data = 0x0;
	*(uint32_t*)(send_data.data) = baud;
	*(uint32_t*)(send_data.data+4) = 0;

	if(check_command(send_data, &recv_data, true) != 0){
		if(DEBUG) printf("err:change buad rate failed\n");
		return 1;
	}
	return SUCCESS;
}

uint8_t g_u8buf1[FLASH_WRITE_SIZE];
ERR_STATUS try_write_flash(void){
	ERR_STATUS errno = 0;
	SLIP_DATA slip_send={
				.msg_type 	= 0,
				.cmd 		= 0,
				.data_size	= 0,
				.extra_data	= 0,
				};
	for(int i=0; i<_load_param.bin_num; i++){
		//get file size	
		int file_size = getFileSize(_load_param.image_path_divid[i]);
		uint32_t u32offset=0;
		if(DEBUG) printf("image size:%d\n", file_size);
		if(file_size < 0)
			return 1;
		
		//flash_spi_attach	
		if(_load_param.chip_type == ESP32 || RUN_STUB){
			if(_flash_spi_attach(0, 0) != 0)
				return 2;
			
			if(DEBUG) printf("flash_spi_attach fin\n");
		}
		
		uart_flush(UART_PORT);
		
		//flash_begin
		if(DEBUG) printf("start:%lld\n", rtc_time_get());
		u32offset = _load_param.flash_addr_divid[i];
		
		int seq=0, foffset=0, size, datalen;
		char massage[100];
		int finished=0;
		size = file_size;			
		if(_flash_begin(size, u32offset)!=0){
			if(DEBUG) printf("_flash_begin failed\n");
			return 3;
		}
		
		uint8_t *data = g_u8buf1;//[FLASH_WRITE_SIZE];

		//char *file_path[100];
		FILE* f = fopen(_load_param.image_path_divid[i], "rb");
	    if (f == NULL) {
	        ESP_LOGW(TAG, "Failed to open file for reading:%s", _load_param.image_path_divid[i]);
			fclose(f);
	        return 3;
	    }
		while(size > 0){

			uart_flush(UART_PORT);
			datalen = size < FLASH_WRITE_SIZE ? size : FLASH_WRITE_SIZE;

			//printf("start load data:%lld\n", rtc_time_get());
			fread(data, datalen, 1, f);

			//update image flash params
			if(seq == 0){
				if(((_load_param.chip_type == ESP32) && (_load_param.flash_addr_divid[i] == 0x1000))\
					|| ((_load_param.chip_type == ESP8266)&&(_load_param.flash_addr_divid[i] == 0x0))){
						data[2] = _load_param.flash_mode & 0xff;
						data[3] = (((_load_param.flash_size << 4) & 0xf0) | (_load_param.flash_speed & 0x0f));
						if(DEBUG) printf("replace image date\n\n");
					}

			}

			//printf("end load data:%lld\n", rtc_time_get());
			memset(data+datalen, 0xff, FLASH_WRITE_SIZE-datalen);

			//printf("start send block data:%lld\n", rtc_time_get());
			if(_flash_block(data, seq, FLASH_WRITE_SIZE)==0){
				;//printf("send block %d success\n", seq);
				//printf("size:%d\n", size);
			}
			else{
				if(DEBUG) printf("send block %d failed\n", seq);
				return 1;
			}

			//printf("end send block data:%lld\n", rtc_time_get());
			size -= FLASH_WRITE_SIZE;
			seq ++;
		}
		
		if(RUN_STUB){
			_flash_begin(0, 0);
			_flash_finish(0);
		}
		
		fclose(f);
	}
	return SUCCESS;
}

uint32_t read_reg(uint32_t addr){
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};

	uint8_t sendData[4], recvData[50];

	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_READ_REG;
	send_data.data_size = 4;
	send_data.extra_data = 0;
	*(uint32_t*)send_data.data = addr;
	if(slip_command(IF_UART, send_data, &recv_data, true) != SUCCESS){
		if(DEBUG) printf("read reg failed\n");
		return -1;
	}
	return recv_data.extra_data;
}

ERR_STATUS write_reg(uint32_t addr, uint32_t value, uint32_t mask, uint32_t delay_us){
	SLIP_DATA send_data={.data=NULL}, recv_data={.data=NULL};

	uint8_t sendData[16], recvData[50];

	send_data.data = sendData;
	recv_data.data = recvData;
	
	send_data.msg_type = 0x00;
	send_data.cmd = ESP_WRITE_REG;
	send_data.data_size = 16;
	send_data.extra_data = 0;
	*(uint32_t*)send_data.data = addr;
	*(uint32_t*)(send_data.data+4) = value;
	*(uint32_t*)(send_data.data+8) = mask;
	*(uint32_t*)(send_data.data+12) = delay_us;

	if(slip_command(IF_UART, send_data, &recv_data, true) != SUCCESS){
		if(DEBUG) printf("write reg failed\n");
		return -1;
	}
	return SUCCESS;
}

//read NO.n word of the ESP3x EFUSE region.
uint32_t read_efuse(uint32_t nth){
	return read_reg(ESP32_EFUSE_REG_BASE + (4*nth));
}

static void _ESP32_set_date_lenth(uint32_t base, uint32_t mosi_bits, uint32_t miso_bits){
	uint32_t SPI_MOSI_DLEN_REG = base + 0x28;
	uint32_t SPI_MISO_DLEN_REG = base + 0x2c;
	if(mosi_bits > 0)
		write_reg(SPI_MOSI_DLEN_REG, mosi_bits-1, 0xffffffff, 0);
	if(miso_bits > 0)
		write_reg(SPI_MISO_DLEN_REG, miso_bits-1, 0xffffffff, 0);

}

static void _ESP8266_set_date_lenth(uint32_t base, uint32_t mosi_bits, uint32_t miso_bits){
	uint32_t SPI_DATA_LEN_REG = base;
	uint32_t SPI_MOSI_BITLEN_S = 17;
	uint32_t SPI_MISO_BITLEN_S = 8;
	uint32_t mosi_mask,miso_mask;

	mosi_mask = mosi_bits==0?0:(mosi_bits-1);
	miso_mask = miso_bits==0?0:(miso_bits-1);

	write_reg(SPI_DATA_LEN_REG, (mosi_mask << SPI_MOSI_BITLEN_S)|\
				(miso_mask << SPI_MISO_BITLEN_S), 0xffffffff, 0 );
}

static void _pad_to(uint8_t *data, uint8_t *raw_data, uint16_t *datalen, uint16_t alignment, uint8_t pad_character){
	int pad_mod = *datalen % alignment;
	if(pad_mod != 0){
		memcpy(data, raw_data, *datalen);
		memset(data+(*datalen), pad_character, alignment-pad_mod);
		*datalen += alignment-pad_mod;
	}
}

static void _wait_done(uint32_t SPI_CMD_REG, uint32_t SPI_CMD_USR){
	for(int i=0; i<100; i++){
		if((read_reg(SPI_CMD_REG) & SPI_CMD_USR) == 0)
			return;
	}
}

uint32_t run_spiflash_command(uint32_t spiflash_command, uint8_t* raw_data, uint16_t datalen, uint32_t read_bits){
	uint32_t SPI_USR_COMMAND = (1 << 31);
	uint32_t SPI_USR_MISO	= (1 << 28);
	uint32_t SPI_USR_MOSI	= (1 << 27);
	uint32_t base;
	uint8_t data[64];

	if(_load_param.chip_type == ESP32)
		base = ESP32_SPI_REG_BASE;
	else if(_load_param.chip_type == ESP8266)
		base = ESP8266_SPI_REG_BASE;
	else
		base = ESP8266_SPI_REG_BASE;

	uint32_t SPI_CMD_REG = base + 0x0;
	uint32_t SPI_USR_REG = base + 0x1c;
	uint32_t SPI_USR1_REG = base + 0x20;
	uint32_t SPI_USR2_REG = base + 0x24;
	uint32_t SPI_W0_REG;
	if(_load_param.chip_type == ESP32)
		SPI_W0_REG = base + ESP32_SPI_W0_OFFS;
	else if(_load_param.chip_type == ESP8266)
		SPI_W0_REG = base + ESP8266_SPI_W0_OFFS;
	else
		SPI_W0_REG = base + ESP8266_SPI_W0_OFFS;


	uint32_t SPI_CMD_USR = (1 << 18);
	uint32_t SPI_USR2_DLEN_SHIFT = 28;

	if(read_bits > 32){
		if(DEBUG) printf("Reading more than 32 bits back from a SPI flash operation is unsupported\n");
		return -1;
	}
	if(datalen > 64){
		if(DEBUG) printf("Writing more than 64 bytes of data with one SPI command is unsupported\n");
		return -1;
	}
	uint32_t data_bits = datalen * 8;
	uint32_t old_spi_usr = read_reg(SPI_USR_REG);
	uint32_t old_spi_usr2 = read_reg(SPI_USR2_REG);
	uint32_t flags = SPI_USR_COMMAND;
	if(read_bits > 0)
		flags |= SPI_USR_MISO;
	if(data_bits > 0)
		flags |= SPI_USR_MOSI;

	if(_load_param.chip_type == ESP32)
		_ESP32_set_date_lenth(base, data_bits, read_bits);
	else
		_ESP8266_set_date_lenth(SPI_USR1_REG, data_bits, read_bits);

	write_reg(SPI_USR_REG, flags, 0xffffffff, 0);
	write_reg(SPI_USR2_REG, (7 << SPI_USR2_DLEN_SHIFT) | spiflash_command, 0xffffffff, 0);

	uint32_t next_reg = SPI_W0_REG;
	if(data_bits == 0)
		write_reg(SPI_W0_REG, 0, 0xffffffff, 0); // clear data register before we read it
	else{
		_pad_to(data, raw_data, &datalen, 4, 0x0);
		for(int i=0; i<datalen / 4; i++){
			write_reg(next_reg, *(uint32_t*)(data+4*i), 0xffffffff, 0);
			next_reg += 4;
		}
	}
	write_reg(SPI_CMD_REG, SPI_CMD_USR, 0xffffffff, 0);

	_wait_done(SPI_CMD_REG, SPI_CMD_USR);

	uint32_t status = read_reg(SPI_W0_REG);
	write_reg(SPI_USR_REG, old_spi_usr, 0xffffffff, 0);
	write_reg(SPI_USR2_REG, old_spi_usr2, 0xffffffff, 0);
	return status;
}

uint32_t get_flash_id_stub(){
	_flash_spi_attach(0, 0);
	_flash_begin(0,0);
	
	uint32_t SPIFLASH_RDID = 0x9F;
	uint8_t data[4] = {0x0};
	return run_spiflash_command(SPIFLASH_RDID, data, 0, 0);
}

uint32_t get_flash_id(void){
	uint32_t data_addr, cmd_addr, cmd_rdid;
	if(_load_param.chip_type == ESP32){
		data_addr = ESP_DATA_ADDR_ESP32;
		cmd_addr = ESP_CMD_ADDR_ESP32;
		cmd_rdid = ESP_CMD_RDID_ESP32;
	}
	else{
		data_addr = ESP_DATA_ADDR_ESP8266;
		cmd_addr = ESP_CMD_ADDR_ESP8266;
		cmd_rdid = ESP_CMD_RDID_ESP8266;
	}
	if(RUN_STUB && _load_param.chip_type == ESP32)
		_flash_spi_attach(0, 0);
	_flash_begin(0,0);
	
	write_reg(data_addr, 0, 0, 0);
	vTaskDelay(1);
	write_reg(cmd_addr, cmd_rdid, cmd_rdid, 0);
	vTaskDelay(1);
	
	return read_reg(data_addr);
}

ERR_STATUS read_mac(char *mac){
	uint8_t mac_raw[8], oui[3];
	uint32_t mac0,mac1,mac3;
	if(_load_param.chip_type == ESP32){
		*(uint32_t*)mac_raw = read_efuse(2);
		*(uint32_t*)(mac_raw+4) = read_efuse(1);
		sprintf(mac, MAC_ADDR_FORMAT, mac_raw[1], mac_raw[0], \
				mac_raw[7], mac_raw[6], mac_raw[5], mac_raw[4]);
		return SUCCESS;
	}
	else if(_load_param.chip_type == ESP8266){
		mac0 = read_reg(ESP_OTP_MAC0);
		mac1 = read_reg(ESP_OTP_MAC1);
		mac3 = read_reg(ESP_OTP_MAC3);

		if(mac3 != 0){
			oui[0] = (mac3 >> 16) & 0xff;
			oui[1] = (mac3 >> 8) & 0xff;
			oui[2] = mac3 & 0xff;
		}
		else if(((mac1 >> 16) & 0xff) == 0){
			oui[0] = 0x18;
			oui[1] = 0xfe;
			oui[2] = 0x34;
		}
		else if(((mac1 >> 16) & 0xff) == 1){
			oui[0] = 0xac;
			oui[1] = 0xd0;
			oui[2] = 0x74;
		}
		else{
			if(DEBUG) printf("err:unknow oui\n");
			return 1;
		}
		sprintf(mac, MAC_ADDR_FORMAT, oui[0], oui[1], oui[2], \
				(mac1>>8)&0xff, mac1&0xff, (mac0>>24)&0xff);
		return SUCCESS;
	}
	return SUCCESS;
}

ERR_STATUS run_stub(void){
	ERR_STATUS errno = 0;
	SLIP_DATA slip_send={
				.msg_type 	= 0,
				.cmd 		= 0,
				.data_size	= 0,
				.extra_data	= 0,
				};
	
	IMAGE image;
	char stub_path[100];
	if(_load_param.chip_type == ESP32)
		strcpy(stub_path, "/sdcard/image/32stub.bin");
	else if(_load_param.chip_type == ESP8266)
		strcpy(stub_path, "/sdcard/image/8266stub.bin");
	
	try_get_Firmware_Image(stub_path, &image);
	
	uint32_t offset, datalen;
	int size;
	int16_t seq=0;
	uint8_t data[ESP_RAM_BLOCK];
	IMAGE_DATA itSegment;

	for(int i=0; i<image.header.segments; i++){
		ESP_LOGI(TAG, "segment%d:  offset:%08x, size:%08x\n",\
					i, image.segment[i].offset, image.segment[i].size);
	}

	//data = (uint8_t*)malloc(ESP_RAM_BLOCK);

	FILE* f = fopen(stub_path, "rb");
    if (f == NULL) {
        ESP_LOGW(TAG, "Failed to open file for reading:%s", stub_path);
		fclose(f);
        return 3;
    }
	if(_load_param.chip_type == ESP32)
		fseek(f, 32, SEEK_SET);
	else
		fseek(f, 16, SEEK_SET);
	
		
	for(int i=0; i<image.header.segments; i++){
		offset = image.segment[i].offset;
		size = image.segment[i].size;

		if(DEBUG) printf("now send segment%d, offset:%04x, size:%04x\n", i, offset, size);
		if(_mem_begin(size, ceil(1.0*size/ESP_RAM_BLOCK), ESP_RAM_BLOCK, offset) != 0){
			if(DEBUG) printf("send mem begin error\n");
			fclose(f);
			return 1;
		}
		
		seq=0;
		
		while(size > 0){
			datalen = size < ESP_RAM_BLOCK ? size : ESP_RAM_BLOCK;
			//get_SegmentData(stub_path, data, foffset+(seq*ESP_RAM_BLOCK), datalen);
			fread(data, datalen, 1, f);
			if(_mem_block(data, seq, datalen)==0){
#if DEBUG
				if(DEBUG) printf("send block %d success; ", seq);
				if(DEBUG) printf("size:%d\n", size);
#endif
			}
			else{
				if(DEBUG) printf("send block %d failed\n", seq);
				fclose(f);
				return 2;
			}
			size -= ESP_RAM_BLOCK;
			seq ++;
		}
		
		fseek(f, 8, SEEK_CUR);
	}
	_mem_finish(image.header.entrypoint);

	uint8_t recvData[1024];
	int len = uart_read_bytes(UART_PORT, recvData, BUF_SIZE, 100 / portTICK_RATE_MS);
	for(int i=0; i<len; i++)
		if(DEBUG) printf("%02x ", *(recvData+i));
	if(DEBUG) printf("\n");

	fclose(f);
	return SUCCESS;
}


ERR_STATUS get_module_result(void){
	uint8_t recvData[1030];
	bool log_start = false, fin_flag = false;
	uint16_t offset=0;
	FILE* f = fopen("/sdcard/mdltest/MDLRST.TXT", "w");
    if (f == NULL) {
        ESP_LOGW(TAG, "Failed to open file for mdl_rst:%s", "/sdcard/mdltest/MDLRST.TXT");
		fclose(f);
        return 1;
    }
	int len=0;
	while(true){
		len = uart_read_bytes(UART_PORT, recvData, BUF_SIZE, 1000 / portTICK_RATE_MS);
		if(len <= 0)
			break;
		recvData[len] = '\0';
		if(DEBUG) printf("%s\n", recvData);
		if((!log_start) && (strstr((char*)recvData, "MODULE_TEST START!!!") != NULL)){
			//offset = strstr((char*)recvData, "MODULE_TEST START!!!");
			log_start = true;
		}

		if(log_start){
			fwrite(recvData, len , 1, f);
			offset = 0;
			if(strstr((char*)recvData, "MODULE_TEST EDN!!!") != NULL){
				fin_flag = true;
				break;
			}
		}
			
	}
	fclose(f);
	if(fin_flag)
		return SUCCESS;
	else
		return 2;
}


void test_image(void){
	_test_image();
}

