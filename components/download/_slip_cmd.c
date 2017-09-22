#include "_slip_cmd.h"
#include <malloc.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "_download_api.h"

#include "soc/soc.h"
#include "soc/rtc.h"


//extern LOAD_PARAM _load_param;


static int _getSlipFrame(uint8_t *recvData, int len, SLIP_DATA *slip_recv){
	uint8_t *p = recvData;
	uint16_t length=0;
	uint8_t slip_temp[BUF_SIZE];
	bool frame_start=false, in_escape=false;
	if(recvData == NULL || len < 8)
		return 1;
	
	for(int i=0; i<len;i++){
		if(!frame_start){
			if(*p == 0xc0){
				frame_start = true;
			}
			else{
				return 2;
			}
		}
		else if(in_escape){
			in_escape = false;
			if(*(p+i) == 0xdc){
				slip_temp[length++] = 0x0c;
			}
			else if(*(p+i) == 0xdd){
				slip_temp[length++] = 0xdb;
			}
			else{
				return 3;
			}
		}
		else if(*(p+i) == 0xdb){
			in_escape = true;
		}
		else if(*(p+i) == 0xc0){
			slip_recv->msg_type = slip_temp[0];
			slip_recv->cmd = slip_temp[1];
			slip_recv->data_size = *(uint16_t*)(slip_temp+2);
			slip_recv->extra_data = *(uint32_t*)(slip_temp+4);
			//if(slip_recv->data == NULL)
			//	slip_recv->data = (uint8_t*)malloc(slip_recv->data_size);
			memcpy(slip_recv->data, slip_temp+8, length-8);
			#if DEBUG
			//print recv cmd
			printf("slip_len:%d, type:%x, cmd:%x, datasize:%d\n", length, slip_recv->msg_type, slip_recv->cmd, slip_recv->data_size);
			#endif
			return 0;
		}
		else{
			slip_temp[length++] = *(p+i);
		}
	}
	return 4;
}

void replaceStr(const char* original, const char* pattern, const char* replacement, char* result){
	int const replen = strlen(replacement);
	int const patlen = strlen(pattern);
	int const orilen = strlen(original);
	
	const char * oriptr, *preptr;	
	const char * patloc;
	char *resprt;
	for (preptr = oriptr = original, resprt=result; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)  
	{ 
		memcpy(resprt, preptr, patloc-preptr);
		resprt += (patloc-preptr);
		memcpy(resprt, replacement, replen);
		resprt += replen;
		preptr = patloc+patlen;
	}  
	memcpy(resprt, preptr, orilen-(preptr-original));
	*(resprt+orilen-(preptr-original)) = '\0';
}  


void replaceChar(const char* original, int orilen, const char* pattern, const char* replacement, char* result, int *reslen){
	int const replen = strlen(replacement);
	int const patlen = strlen(pattern);
	
	*reslen=0;
	for(int i=0; i<orilen; ){
		if(strncmp(original+i, pattern, patlen) != 0){
			*(result+*reslen) = *(original+i);
			(*reslen)++;
			i++;
		}
		else{
			memcpy(result+*reslen, replacement, replen);	
			*reslen += replen;
			i += patlen;
		}
	}
	
}

uint8_t g_u8buf[2][FLASH_WRITE_SIZE+1000];

ERR_STATUS slip_command(IF_TYPE if_type, SLIP_DATA slip_send, SLIP_DATA *slip_recv,bool wait_flag){
	LOAD_PARAM *load_param = getLoadParams();
	//uint8_t sendData[FLASH_WRITE_SIZE+1000], recvData[BUF_SIZE], sendData2[FLASH_WRITE_SIZE+1000];
	uint8_t *sendData = g_u8buf[0], recvData[BUF_SIZE], *sendData2 = g_u8buf[1];
	//sendData = (uint8_t*)malloc(8+slip_send.data_size+2);
	memset(sendData, 0, 10+slip_send.data_size+1);	
	//*sendData = 0xc0;
	//*(sendData+8+slip_send.data_size+1) = 0xc0;
	*(sendData+1) = 0x00;
	*(sendData+2) = (slip_send.cmd & 0xff);
	*(sendData+3) = (slip_send.data_size & 0xff);
	*(sendData+4) = ((slip_send.data_size>>8) & 0xff);
	*(uint32_t*)(sendData+5) = slip_send.extra_data;
	memcpy(sendData+9, slip_send.data, slip_send.data_size);
	
	int len1=8+slip_send.data_size+1, len2=0;
	//printf("start replace data:%lld\n", rtc_time_get());
	replaceChar((const char*)sendData, len1, "\xdb", "\xdb\xdd", (char*)sendData2, &len2);
	replaceChar((const char*)sendData2, len2, "\xc0", "\xdb\xdc", (char*)sendData, &len1);
	//printf("end replace data:%lld\n", rtc_time_get());
	sendData[0] = 0xc0;
	sendData[len1] = 0xc0;
	len1++;

#if DEBUG
	// print send raw date
	printf("len:%d\n", len1);
	if(slip_send.cmd != ESP_MEM_DATA && slip_send.cmd != ESP_FLASH_DATA){
		
		for(int i=0; i<len1 && i<60; i++)
			printf("%02x ", *(sendData+i));
		printf("\n");
	}
#endif
	int len=0;
	if(IF_UART==if_type){
		//printf("start send data:%lld\n", rtc_time_get());
		uart_write_bytes(UART_PORT, (const char*)sendData, len1);
		//printf("end send data:%lld\n", rtc_time_get());
		if(!wait_flag){
			return SUCCESS;
		} 
		//wait for response, total 10 times
		//printf("start recv and deal data:%lld\n", rtc_time_get());
		
		for(int i=0; i<10; i++){
			if(RUN_STUB){
				len += uart_read_bytes(UART_PORT, recvData+len, BUF_SIZE, load_param->port_timeout_ms / portTICK_RATE_MS);
			}
			else{
				len = 0;
				len = uart_read_bytes(UART_PORT, recvData, BUF_SIZE, load_param->port_timeout_ms / portTICK_RATE_MS);
			}
#if DEBUG
			printf("%d:", i);
			//print recv raw data
			for(int i=0; i<len; i++)
				printf("%02x ", *(recvData+i));
			printf("\n");
#endif
			
			//printf("%d recv:%lld\n", i, rtc_time_get());
			if(len < 8) continue;
			//printf("end recv data start deal:%lld\n", rtc_time_get());
			if(_getSlipFrame(recvData, len, slip_recv) != 0) continue;
			//printf("end deal data:%lld\n", rtc_time_get());
			if(slip_recv->msg_type != 0x01) continue;
			if(slip_recv->data[0] != 0){
				printf("get some failed:%02x\n", slip_recv->data[1]);
				return 2;
			}
			if(slip_recv->cmd == slip_send.cmd){
				
				return SUCCESS;
			}
		}		
	}
	return ERR_FW_DOWNLOAD;
}

ERR_STATUS check_command(SLIP_DATA slip_send, SLIP_DATA *slip_recv, bool wait_flag){
	uint8_t status_bytes[STATUS_BYTES_LENGTH];
	uint8_t status_byte_len = 2;
	LOAD_PARAM *load_param = getLoadParams();
	if(RUN_STUB){
		status_byte_len = 2;
	}
	else{
		status_byte_len = (load_param->chip_type+1)*2;
	}
	if(slip_command(IF_UART, slip_send, slip_recv, wait_flag) != 0){
		printf("call cmd failed\n");
		return ERR_FW_DOWNLOAD;
	}
	if(wait_flag == false)
		return SUCCESS;

	if(RUN_STUB){
		return SUCCESS;
	}
	if(slip_recv->data_size < status_byte_len && !RUN_STUB){
		printf("recv data err\n");
		return ERR_FW_DOWNLOAD;
	}
	memcpy(status_bytes, (slip_recv->data + slip_recv->data_size - status_byte_len), status_byte_len);
	if(status_bytes[0] != 0){
		printf("status_bytes err\n");
		return ERR_FW_DOWNLOAD;
	}
	
	return SUCCESS;
}


