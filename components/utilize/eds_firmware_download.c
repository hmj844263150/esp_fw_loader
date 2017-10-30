#include "eds_firmware_download.h"
#include "_download_api.h"
#include "_sd_api.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "eds_driver_init.h"
#include "soc/rtc.h"
#include "lcd_api.h"
#include "lcd_func.h"


int module_test_cpp();


ERR_STATUS eds_load_ram(void){
	ERR_STATUS errno = 0;

	//sync and conn chip
	if(eds_connect() != 0)		
		return ERR_FD_CONN;

	LCD_drawCentreString("connect success", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
	if(DEBUG) printf("connect to chip success\n");

	if(try_load_ram() != SUCCESS){
		if(DEBUG) printf("load to ram failed\n");
		return ERR_FW_DOWNLOAD;
	}
	if(DEBUG) printf("load to ram fin\n");
	
	VERIFY_PARAM *verify_param = getVerifyParams();
	if(verify_param->verify_en){
		if(verify_ram()!= SUCCESS)
			return ERR_FW_DOWNLOAD;
	}
	
	return SUCCESS;
}

ERR_STATUS eds_connect(void){
	LOAD_PARAM *load_param = getLoadParams();
	load_param->port_timeout_ms = 10;
	//init buad rate for sync
	uart_set_baudrate(UART_PORT, 115200);
	
	return try_connect(false);

}

ERR_STATUS eds_loadParam_init(void){
	SD_PARAM *sd_params = getSdParams();
	//LOAD_PARAM *load_param = getLoadParams();
	if(getParam_fromSD(sd_params) != 0){
		if(DEBUG) printf("get param failed\n");
		return ERR_FW_DOWNLOAD;
	}
	//sys_load_param = load_param;
	if(DEBUG) printf("wifi_ssid:%s, wifi_pwd:%s\n", sd_params->wifi_param->wifi_ssid, sd_params->wifi_param->wifi_pwd);
	if(DEBUG) printf("chip_type:%d\n__download_mode:%d\n__flash_size:%d\n__flash_speed:%d\n__flash_mode:%d\n__image_path:%s\n", 
			(int)sd_params->load_param->chip_type,(int)sd_params->load_param->download_mode, \
			(int)sd_params->load_param->flash_size, (int)sd_params->load_param->flash_speed, \
			(int)sd_params->load_param->flash_mode, sd_params->load_param->image_path);

	for(int i=0; i<sd_params->load_param->bin_num; i++){
		if(DEBUG) printf("bin:%s  addr:0x%x\n", sd_params->load_param->image_path_divid[i], sd_params->load_param->flash_addr_divid[i]);
	}

	if(DEBUG) printf("bin_ver:%s, Tool_ver:%s, becthNO:%s\n", sd_params->verify_param->bin_version,\
				sd_params->verify_param->tool_version, sd_params->verify_param->becth_no);
	if(sd_params->verify_param->verify_en){
		if(DEBUG) printf("verify value:%s\nverify tmo:%d\nverify buad:%d\n", \
				sd_params->verify_param->verify_value, 
				sd_params->verify_param->verify_tmo, 
				sd_params->verify_param->verify_buadrate);
	}

	char prtStr[100];

	sprintf(prtStr, "Bin ver: %s", sd_params->verify_param->bin_version);
	LCD_drawString(prtStr, LCD_LOCATION_MSG_1, 2, LCD_WORK_AREA_COLOR, LCD_FONT_COLOR);
	sprintf(prtStr, "Tool ver: %s", sd_params->verify_param->tool_version);
	LCD_drawString(prtStr, LCD_LOCATION_MSG_2, 2, LCD_WORK_AREA_COLOR, LCD_FONT_COLOR);
	sprintf(prtStr, "Becth NO: %s", sd_params->verify_param->becth_no);
	LCD_drawString(prtStr, LCD_LOCATION_MSG_3, 2, LCD_WORK_AREA_COLOR, LCD_FONT_COLOR);
	return SUCCESS;
}

ERR_STATUS eds_write_flash(void){
	if(try_write_flash() != 0){
		return ERR_FW_DOWNLOAD;
	}
	return SUCCESS;
}

void eds_LCD_showChipInfo(LOAD_PARAM *load_param){
	char prtStr[100];
	if((load_param->flash_id & 0xff) == 0xA1)
		sprintf(prtStr, "Flash: %s,", "FM");
	else if((load_param->flash_id & 0xff) == 0xc8)
		sprintf(prtStr, "Flash: %s,", "GD");
	else if((load_param->flash_id & 0xff) == 0x9d)
		sprintf(prtStr, "Flash: %s,", "ISSI");
	else if((load_param->flash_id & 0xff) == 0xc2)
		sprintf(prtStr, "Flash: %s,", "KH");
	else if((load_param->flash_id & 0xff) == 0xef)
		sprintf(prtStr, "Flash: %s,", "WB");
	else
		sprintf(prtStr, "Flash: %s,", "UNKNOW");
	
	if(((load_param->flash_id >> 8)&0xff) == 0x40)
		sprintf(prtStr+strlen(prtStr), "QUAD,");
	else if(((load_param->flash_id >> 8)&0xff) == 0x30)
		sprintf(prtStr+strlen(prtStr), "DUAL,");

	if(((load_param->flash_id >> 16)&0xff) < 0x09)
		sprintf(prtStr+strlen(prtStr), "256Kbit");
	else if(((load_param->flash_id >> 16)&0xff) < 0x10)
		sprintf(prtStr+strlen(prtStr), "512Kbit");
	else
		sprintf(prtStr+strlen(prtStr), "%dMbit", (int)pow(2, (int)(((load_param->flash_id >> 16)&0xff)-0x11)));
	LCD_drawString(prtStr, LCD_LOCATION_MSG_4, 2, LCD_WORK_AREA_COLOR, LCD_FONT_COLOR);

	if(load_param->chip_type == ESP32)
		sprintf(prtStr, "Device type: ESP32");
	else if(load_param->chip_type == ESP8266)
		sprintf(prtStr, "Device type: ESP8266");
	else if(load_param->chip_type == ESP8285)
		sprintf(prtStr, "Device type: ESP8285");
	LCD_drawString(prtStr, LCD_LOCATION_MSG_5, 2, LCD_WORK_AREA_COLOR, LCD_FONT_COLOR);
	sprintf(prtStr, "mac: %s", load_param->chip_mac);
	LCD_drawString(prtStr, LCD_LOCATION_MSG_6, 2, LCD_WORK_AREA_COLOR, LCD_FONT_COLOR);
}


ERR_STATUS _module_test(char* mdl_logs){
	LCD_drawCentreString("loading...", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
	if(try_load_ram() != SUCCESS){
		if(DEBUG) printf("load to ram failed\n");
		return ERR_MODULE_TEST;
	}
	if(DEBUG) printf("load to ram fin\n");
	LCD_drawCentreString("testing...", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
	if(get_module_result() != SUCCESS){
		if(DEBUG) printf("get log failed\n");
		return ERR_MODULE_TEST;
	}

	if(module_test_cpp(mdl_logs) != SUCCESS){
		if(DEBUG) printf("module test failed\n");
		return ERR_MODULE_TEST;
	}

	return SUCCESS;
}


ERR_STATUS eds_firmware_download(void){
	ERR_STATUS errno = 0;
	if(DEBUG) printf("start sync:%lld\n", rtc_time_get());

	//sync and conn chip
	if(eds_connect() != 0)		
		return ERR_FD_CONN;
	
	if(DEBUG) printf("end sync:%lld\n", rtc_time_get());
	LCD_drawCentreString("connect success", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
	if(DEBUG) printf("connect to chip success\n");

	//load param from SD card
	LOAD_PARAM *load_param = getLoadParams();


	//load stub bin
	if(RUN_STUB){
		if(DEBUG) printf("start run stub:%lld\n", rtc_time_get());
		if(run_stub() != SUCCESS){
			if(DEBUG) printf("run stub failed\n");
			return ERR_FW_DOWNLOAD;
		}
		if(DEBUG) printf("end run stub:%lld\n", rtc_time_get());
	}

	
	//erase flash if use stub
	if(RUN_STUB){
		if(DEBUG) printf("erase start:%lld\n", rtc_time_get());
		if(erase_flash() != SUCCESS){
			if(DEBUG) printf("erase flash failed\n");
			return ERR_FW_DOWNLOAD;
		}
		if(DEBUG) printf("erase fin:%lld\n", rtc_time_get());
	}
	
	
	//improve buad rate for load image
	if(DEBUG) printf("start change buad:%lld\n", rtc_time_get());
	
	if(load_param->chip_type == ESP32 || RUN_STUB){
		if(change_baud(115200*16) != 0){
			if(DEBUG) printf("change buad failed\n");
			return ERR_FW_DOWNLOAD;
		}
		else{
			if(DEBUG) printf("change buad success\n");
			uart_set_baudrate(UART_PORT, 115200*16);
		}
	}
	if(DEBUG) printf("end change buad:%lld\n", rtc_time_get());
	
	
	if(read_mac(load_param->chip_mac) != SUCCESS){
		if(DEBUG) printf("get chip mac failed\n");
		return ERR_FW_DOWNLOAD;
	}
	if(DEBUG) printf("chip mac:%s\n", load_param->chip_mac);
	
	//uint32_t flash_id = get_flash_id_stub();
	load_param->flash_id = get_flash_id();
	if(load_param->flash_id == -1){
		if(DEBUG) printf("get flash id failed\n");
		return ERR_FW_DOWNLOAD;
	}
	
	if(DEBUG) printf("flash id:%08x\n", load_param->flash_id);
	eds_LCD_showChipInfo(load_param);

	if(DEBUG) printf("start load:%lld\n", rtc_time_get());


	if(DEBUG) printf("start to load to flash\n");
	LCD_drawCentreString("Downloading...", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
	if(eds_write_flash() != SUCCESS){
		if(DEBUG) printf("write flash failed\n");
		return ERR_FW_DOWNLOAD;
	}

	if(DEBUG) printf("end load:%lld\n", rtc_time_get());

	VERIFY_PARAM *verify_param = getVerifyParams();
	if(verify_param->verify_en){
		if(verify_flash()!= SUCCESS)
			return ERR_FW_DOWNLOAD;
	}
	
	
	return SUCCESS;
}

ERR_STATUS eds_module_test(char* mdl_logs){
	//sync and conn chip
	if(eds_connect() != 0)		
		return ERR_FD_CONN;
	
	LCD_drawCentreString("connect success", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
	if(DEBUG) printf("connect to chip success\n");
	
	if(DEBUG) printf("start to mdl testing\n");
	if(_module_test(mdl_logs) != SUCCESS){
		if(DEBUG) printf("fcc test failed\n");
		return ERR_MODULE_TEST;
	}
	return SUCCESS;
}

