#include "_sd_api.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>  
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"

static const char* TAG = "SD card";
/*
#define ESP_CHIP_TYPE		0
#define ESP_DOWNLOAD_MODE	1
#define ESP_FLASH_FREQ		2
#define ESP_FLASH_MODE		3
#define ESP_FLASH_SIZE		4
#define ESP_FLASH_PATH		5
#define ESP_FLASH_ADDR		6
*/
typedef enum{
	SD_WIFI_SSID 		= 0,
	SD_WIFI_PWD,
	SD_CHIP_TYPE,
	SD_ESP_DOWNLOAD_MODE,
	SD_RAM_IMAGE_PATH,
	SD_FLASH_FREQ,
	SD_FLASH_MODE,
	SD_FLASH_SIZE,
 	SD_FLASH_BIN_NUM,
 	SD_FLASH_BIN_NAME,
 	SD_FLASH_BIN_ADDR,
 	
	SD_VERIFY_BIN_VER,
	SD_VERIFY_TOOL_VER,
	SD_VERIFY_BECTH_NO,
	SD_VERIFY_TOOL_NO,
 	SD_VERIFY_EN,
 	SD_VERIFY_VALUE,
 	SD_VERIFY_TMO,
 	SD_VERIFY_BUAD,
 	SD_PARAM_NUM = 18,
}PARAM_TYPE;

char *param_type[] = {"WIFI_SSID", "WIFI_PWD",  "CHIP_TYPE", "ESP_DOWNLOAD_MODE", \
					 	"RAM_IMAGE_PATH", "FLASH_FREQ", "FLASH_MODE", "FLASH_SIZE", \
					 	"FLASH_BIN_NUM", "FLASH_BIN_NAME", "FLASH_BIN_ADDR", "VERIFY_BIN_VER", \
					 	"VERIFY_TOOL_VER", "VERIFY_BECTH_NO", "VERIFY_TOOL_NO", \
					 	"VERIFY_EN", "VERIFY_VALUE", "VERIFY_TMO", "VERIFY_BUAD"};

static 
bool get_Data_from_info(char *srcStr, char *pkeyword, char *pbuf, int len){
	char *ptStr, *begin, *end;
	ptStr = srcStr;
	begin = (char *)strstr(ptStr, pkeyword);
	if(NULL == begin){
		//ESP_LOGW(TAG, "get data err:not get key word\n");
		return false;
	}
	begin += strlen(pkeyword);
	while(*begin != '\0'){
		if(*begin == ' ')
			begin ++;
		else
			break;
	}
	if(NULL == begin){
		ESP_LOGW(TAG, "key word is empty\n");
		return false;
	}
	end = begin;
	while(*end != '\0'){
		if((*end == ' ') || (*end == '\r') || (*end == '\n') || (*end == '\t'))
			break; 
		else
			end ++;;
	}
	if(end-begin>len-1){
		ESP_LOGW(TAG, "get data err:the data is too long\n");
		return false;
	}
	strncpy(pbuf, begin, end-begin);
	pbuf[end-begin]='\0';
	return true;
}

/* 
 * ascii to num
 * */  
static int c2i(char ch){  
	if(ch >= '0' && ch <= '9')
		return ch - '0';  

	if(ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;

	if(ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;

	return -1;
}  
  
/* 
 * transfer hex str to dec num
 * */  
static int hex2dec(char *hex){  
	int len;  
	int num = 0;  
	int temp;  
	int bits;  
	int i;
	len = strlen(hex)-2;
	if(*(hex+1) == 'x'){
		hex = hex+2;
	}

	for (i=0, temp=0; i<len; i++, temp=0){  
        temp = c2i( *(hex + i) );  
        bits = (len - i - 1) * 4;  
        temp = temp << bits;  
        num = num | temp;  
	}  
	return num;  
}  


ERR_STATUS getParam_fromSD(SD_PARAM *param){
	ESP_LOGW(TAG, "Opening file");
	// Check if destination file exists before renaming
    struct stat st;
    if (stat("/sdcard/config/settings.txt", &st) != 0) {
		printf("not find config text\n");
        return 1;
    }
    FILE* f = fopen("/sdcard/config/settings.txt", "r");
    if (f == NULL) {
        ESP_LOGW(TAG, "Failed to open file for reading");
        return 2;
    }
	char line[200], msg[100];
	uint16_t tmp_flag=0x0;
	uint8_t bin_num=0;
	while(!feof(f)){
		memset(line, 0, 200);
		
	    fgets(line, sizeof(line), f);
		//ESP_LOGD(TAG, "%s\n", line);
		if(strncmp(line, "#define", 7) != 0)
			continue;
				
		for(int i=0; i<SD_PARAM_NUM; i++){
			if(tmp_flag & (1<<i)) //this param already get
				continue;
			memset(msg, 0, 100);
			if(get_Data_from_info(line, param_type[i], msg, sizeof(msg))){
				printf("get %s:%s\n", param_type[i], msg);
				tmp_flag |= (1<<i);
				switch(i){
					case SD_WIFI_SSID:
						strcpy(param->wifi_param->wifi_ssid, msg);
						break;
					case SD_WIFI_PWD:
						strcpy(param->wifi_param->wifi_pwd, msg);
						break;
					case SD_CHIP_TYPE:
						if(strcmp(msg, "ESP8266") == 0)
							param->load_param->chip_type = ESP8266;
						else if(strcmp(msg, "ESP32") == 0)
							param->load_param->chip_type = ESP32;
						else if(strcmp(msg, "ESP8285") == 0)
							param->load_param->chip_type = ESP8285;
						else
							printf("setting msg error:CHIP_TYPE\n");
						break;
					case SD_ESP_DOWNLOAD_MODE:
						if(strcmp(msg, "1") == 0)
							param->load_param->download_mode = LOAD_RAM;
						else if(strcmp(msg, "2") == 0)
							param->load_param->download_mode = LOAD_FLASH;
						else if(strcmp(msg, "3") == 0)
							param->load_param->download_mode = MDL_TEST;
						else
							printf("setting msg error:ESP_DOWNLOAD_MODE\n");
						break;
					case SD_RAM_IMAGE_PATH:
						strcpy(param->load_param->image_path, "/sdcard");
						memcpy(param->load_param->image_path+strlen("/sdcard"), msg+1, strlen(msg));
						break;
					case SD_FLASH_FREQ:
						if(strcmp(msg, "40m") == 0)
							param->load_param->flash_speed = FLASH_SPEED_40M;
						else if(strcmp(msg, "26m") == 0)
							param->load_param->flash_speed = FLASH_SPEED_26M;
						else if(strcmp(msg, "20m") == 0)
							param->load_param->flash_speed = FLASH_SPEED_20M;
						else if(strcmp(msg, "80m") == 0)
							param->load_param->flash_speed = FLASH_SPEED_80M;
						else
							printf("setting msg error:FLASH_FREQ\n");
						break;
					case SD_FLASH_MODE:
						if(strcmp(msg, "qio") == 0)
							param->load_param->flash_mode = FLASH_MODE_QIO;
						else if(strcmp(msg, "qout") == 0)
							param->load_param->flash_mode = FLASH_MODE_QOUT;
						else if(strcmp(msg, "dio") == 0)
							param->load_param->flash_mode = FLASH_MODE_DIO;
						else if(strcmp(msg, "dout") == 0)
							param->load_param->flash_mode = FLASH_MODE_DOUT;
						else if(strcmp(msg, "fastrd") == 0)
							param->load_param->flash_mode = FLASH_MODE_FASTRD;
						else
							printf("setting msg error\n");
						break;
					case SD_FLASH_SIZE:
						if(strcmp(msg, "1MB") == 0)
							param->load_param->flash_size = FLASH_SIZE_32_8M;
						else if(strcmp(msg, "2MB") == 0)
							param->load_param->flash_size = FLASH_SIZE_32_16M;
						else if(strcmp(msg, "4MB") == 0)
							param->load_param->flash_size = FLASH_SIZE_32_32M;
						else if(strcmp(msg, "8MB") == 0)
							param->load_param->flash_size = FLASH_SIZE_32_64M;
						else if(strcmp(msg, "16MB") == 0)
							param->load_param->flash_size = FLASH_SIZE_32_128M;
						else if(strcmp(msg, "4Mb") == 0)
							param->load_param->flash_size = FLASH_SIZE_8266_4M;
						else if(strcmp(msg, "2Mb") == 0)
							param->load_param->flash_size = FLASH_SIZE_8266_2M;
						else if(strcmp(msg, "8Mb") == 0)
							param->load_param->flash_size = FLASH_SIZE_8266_8M;
						else if(strcmp(msg, "16Mb") == 0)
							param->load_param->flash_size = FLASH_SIZE_8266_16M;
						else if(strcmp(msg, "32Mb") == 0)
							param->load_param->flash_size = FLASH_SIZE_8266_32M;
						else if(strcmp(msg, "16Mb-C1") == 0)
							param->load_param->flash_size = FLASH_SIZE_8266_16Mb_C1;
						else if(strcmp(msg, "32Mb-C1") == 0)
							param->load_param->flash_size = FLASH_SIZE_8266_32Mb_C1;
						else
							printf("setting msg error:FLASH_SIZE\n");
						break;
					case SD_FLASH_BIN_NUM:
						bin_num = param->load_param->bin_num = atoi(msg);
						break;
					case SD_FLASH_BIN_NAME:
						sprintf(param->load_param->image_path_divid[param->load_param->bin_num - bin_num], "/sdcard/image/%s", msg);

						break;
					case SD_FLASH_BIN_ADDR:
						param->load_param->flash_addr_divid[param->load_param->bin_num - bin_num] = hex2dec(msg);
						if(bin_num > 0){
							tmp_flag &= 0xf9ff;
							bin_num --;
						}
						break;
					case SD_VERIFY_BIN_VER:
						strcpy(param->verify_param->bin_version, msg);
						break;
					case SD_VERIFY_TOOL_VER:
						strcpy(param->verify_param->tool_version, msg);
						break;
					case SD_VERIFY_BECTH_NO:
						strcpy(param->verify_param->becth_no, msg);
						break;
					case SD_VERIFY_TOOL_NO:
						param->verify_param->tool_no = atoi(msg);
						break;
					case SD_VERIFY_EN:
						if((strcmp(msg, "TRUE")==0)||(strcmp(msg, "True")==0)||(strcmp(msg, "true")==0))
							param->verify_param->verify_en= true;
						else
							param->verify_param->verify_en= false;
						break;
					case SD_VERIFY_VALUE:
						strcpy(param->verify_param->verify_value, msg);
						break;
					case SD_VERIFY_TMO:
						param->verify_param->verify_tmo = atoi(msg);
						break;
					case SD_VERIFY_BUAD:
						param->verify_param->verify_buadrate = atoi(msg);
						break;
					default:
						break;

				}
				
				break;
			}
		}
		
	}
	fclose(f);
	if((tmp_flag & 0x0fff) == 0x0fff)
		printf("get all parameters\n");
	return SUCCESS;
}


ERR_STATUS get_Image(char *image_path, IMAGE *image, uint8_t chip_type){
	ESP_LOGW(TAG, "%s, %d", __func__, __LINE__);
	printf("get path from param:%s\n", image_path);

    FILE* f = fopen(image_path, "rb");
    if (f == NULL) {
        ESP_LOGW(TAG, "Failed to open file for reading");
        return 2;
    }
	char cTemp[16];
	fread(cTemp, 1, 8, f);
	
	image->header.magic = cTemp[0];
	image->header.segments = cTemp[1];
	image->header.flash_mode = cTemp[2];
	image->header.flash_size_freq = cTemp[3];
	image->header.entrypoint = *(uint32_t*)(cTemp+4);
	
	//ESP_LOGI(TAG, "%02x, %02x, %02x, %02x, %04x\n", image->header.magic, image->header.segments,
	//			image->header.flash_mode, image->header.flash_size_freq, image->header.entrypoint);
	
	if((image->header.magic != ESP_IMAGE_MAGIC) || (image->header.segments>MAX_SEGMENT_SIZE)){
		ESP_LOGW(TAG, "image data error\n");	
		fclose(f);
		return 1;
	}
	//read next 16 bytes data, ESP32 image header contains unknown flags. Possibly this image is from a newer version
	//unused now
	if(chip_type == ESP32)
		fread(cTemp, 1, 16, f);

	//read segments into a link
	uint32_t offset=0, size=0;
	for(int i=0; i<image->header.segments; i++){
		fseek(f, size, SEEK_CUR);
		fread(cTemp, 1, 8, f);
		if(feof(f)){
			ESP_LOGW(TAG, "err:End of file reading segment\n");
			fclose(f);
			return 2;
		}
			
		offset = *(uint32_t*)(cTemp);
		size = *(uint32_t*)(cTemp+4);
		ESP_LOGD(TAG, "offset%d:%08x, size%d:%08x\n", i, offset, i, size);
		if(offset>0x40200000 || offset<0x3ffe0000 || size > 65536)
			ESP_LOGW(TAG, "WARNING: Suspicious segment 0x%x, length %d\n", offset, size);
		
		image->segment[i].offset = offset;
		image->segment[i].size = size;
	}
	if(feof(f)){
		ESP_LOGW(TAG, "err:End of file reading checksum\n");
		fclose(f);
		return 3;
	}
	fread(cTemp, 1, 16, f);
	memcpy(image->checksum, cTemp, 16);
	
	fclose(f);
	return 0;
}


ERR_STATUS get_SegmentData(char *fdPath, uint8_t *pdata, uint32_t offset, uint32_t size){
    FILE* f = fopen(fdPath, "rb");
    if (f == NULL) {
        ESP_LOGW(TAG, "Failed to open file for reading");
		fclose(f);
        return 2;
    }
	fseek(f, offset, SEEK_SET);
	fread(pdata, size, 1, f);
	
	fclose(f);
	return 0;
}

int getFileSize(char *fdPath){
	/*
	struct stat statbuf;  
    stat(fdPath,&statbuf);  
    int size=statbuf.st_size; 
    */
  	FILE* f = fopen(fdPath, "rb");
    if (f == NULL) {
        ESP_LOGW(TAG, "Failed to open file for reading");
		fclose(f);
        return 0;
    }
	fseek(f, 0, SEEK_END);
	int size = ftell(f);  
	fclose(f);
     
	return size;
}


ERR_STATUS write_to_SDcard(char *time, char *mac, char *bin_ver, char *becth_no, bool res){
	FILE* f = fopen("/sdcard/record.txt", "a");
	if (f == NULL) {
        ESP_LOGW(TAG, "Failed to open file for recording");
		fclose(f);
        return 2;
    }
	char record[200];
	sprintf(record, "%s: MAC:%s, bin_ver:%s, becth_no:%s, ", time, mac, bin_ver, becth_no);
	if(res)
		sprintf(record+strlen(record), "success\n");
	else
		sprintf(record+strlen(record), "failed\n");
	
	fwrite(record,strlen(record), 1, f);
	fclose(f);
	printf("record finish\n");
	return SUCCESS;
}


