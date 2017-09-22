/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "eds_config.h"
#include "eds_driver_init.h"
#include "eds_module_init.h"
#include "eds_result.h"
#include "eds_firmware_download.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"

//LCD driver
#include "lcd_func.h"
#include "driver/gpio.h"
#include "_sd_api.h"
#include "lcd_api.h"

#include "eds_module_test.h"


#define ASSERT_CHECK(x)	err_no=x;if(err_no!=SUCCESS){ \
					printf("get a err no:%d\n", err_no);goto ERR_OVER;}

static SemaphoreHandle_t xSemConnet = NULL;
/*
FreeRTOS event group to signal when we are 
connected & ready to make a request 
*/
static EventGroupHandle_t download_event_group;
const int CONNECTED_BIT = BIT0;
const int RTC_INIT_BIT = BIT1;
const int TIME_SYNC_BIT = BIT2;


//extern LOAD_PARAM sys_load_param;

#define GPIO_OUTPUT_PIN_SEL ((uint64_t)(((uint64_t)1)<<PIN_LED))
#define GPIO_INPUT_PIN_SEL ((uint64_t)(((uint64_t)1)<<PIN_KEY))


void key_io_init(void){	
	gpio_config_t io_conf;    
	//disable interrupt    
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;    
	//set as output mode    
	io_conf.mode = GPIO_MODE_OUTPUT;    
	//bit mask of the pins that you want to set,e.g.GPIO33   
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;    
	//disable pull-down mode    
	io_conf.pull_down_en = 0;    
	//disable pull-up mode    
	io_conf.pull_up_en = 0;    
	//configure GPIO with the given settings    
	gpio_config(&io_conf);

	//interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //bit mask of the pins, use GPIO32
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull_down mode
    io_conf.pull_up_en = 0;
	io_conf.pull_down_en = 1;
    gpio_config(&io_conf);
    
}


void keyPush(void){
	xSemaphoreGive(xSemConnet);
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(download_event_group, CONNECTED_BIT);
        printf("got ip\n");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(download_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
	WIFI_PARAM *wifi_param = getWifiParams();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	/*
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = (uint8_t*)wifi_param->wifi_ssid,//EXAMPLE_WIFI_SSID,
            .password = (uint8_t*)wifi_param->wifi_pwd,//EXAMPLE_WIFI_PASS,
        },
    };
    */
    wifi_config_t wifi_config={.sta={.ssid="",.password="",},};
	memcpy((char*)wifi_config.sta.ssid, wifi_param->wifi_ssid, strlen(wifi_param->wifi_ssid)+1);
	memcpy((char*)wifi_config.sta.password, wifi_param->wifi_pwd, strlen(wifi_param->wifi_pwd)+1);
    printf( "Setting WiFi configuration SSID %s; PWD:%s\n", wifi_config.sta.ssid, wifi_config.sta.password);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static ERR_STATUS esd_sys_init(void){
	if(eds_driver_init() != SUCCESS){
		printf("eds_driver_init failed\n");
		return 1;
	}
	//initial download param(get from SD card)
	eds_loadParam_init();
	xEventGroupSetBits(download_event_group, RTC_INIT_BIT);
	
	initialise_wifi();
	return SUCCESS;
}


void eds_task(void *pvParameter)
{	
	bool opt_flag = false;
	char mdl_logs[1024];
	
	if (xSemConnet == NULL)
        xSemConnet = xSemaphoreCreateBinary();
	
	ERR_STATUS err_no = 0;
	int module_status = MS_WAIT_MODULE;
	
	LCD_stageChange(MS_WAIT_MODULE, false, 0);
	ASSERT_CHECK(eds_module_init());

	SD_PARAM *sd_param = getSdParams();
	while(1)
	{
		switch(module_status){
			case MS_WAIT_MODULE:
				if(xSemaphoreTake(xSemConnet, 100 / portTICK_PERIOD_MS) == pdTRUE){
					if(sd_param->load_param->download_mode == LOAD_FLASH)
						module_status = MS_FW_DOWNLOAD;
					else if(sd_param->load_param->download_mode == MDL_TEST)
						module_status = MS_MDL_TEST;
				}
				break;
			case MS_FW_DOWNLOAD:
				LCD_stageChange(MS_FW_DOWNLOAD, false, sd_param->verify_param->tool_no);
				if(eds_firmware_download() != SUCCESS)
				{
					opt_flag = false;
					printf("download not success, but you can try again\n");
					LCD_drawCentreString("load failed", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
					module_status = MS_FW_CHECK;
				}
				else{
					opt_flag = true;
					LCD_drawCentreString("load success", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
					module_status = MS_FW_CHECK;
				}
				break;

			case MS_MDL_TEST:
				LCD_stageChange(MS_MDL_TEST, false, sd_param->verify_param->tool_no);
				if(eds_module_test(mdl_logs) != SUCCESS)
				{
					opt_flag = false;
					printf("module test failed, detail in logs\n");
					LCD_drawCentreString("test failed", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
					module_status = MS_FW_CHECK;
				}
				else{
					opt_flag = true;
					LCD_drawCentreString("test success", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
					module_status = MS_FW_CHECK;
				}
				printf("%s", mdl_logs);
				break;
			case MS_FW_CHECK:
				ASSERT_CHECK(eds_result_opration(sd_param, opt_flag));
				LCD_stageChange(MS_FW_CHECK, opt_flag, sd_param->verify_param->tool_no);
				LCD_drawCentreString("record finish", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
				module_status = MS_WAIT_MODULE;
				break;
			default:
				printf("get an error case of module_status:%d\n", module_status);
				goto ERR_OVER;
				break;
		}
	}

ERR_OVER:
	vTaskDelete(NULL);
}


void key_task(void *pvParameter){
	if (xSemConnet == NULL)
        xSemConnet = xSemaphoreCreateBinary();
	
	key_io_init();
	
	int stage=0, count=0;
	while(1){
		switch(stage){
			case 0:
				if(1 == gpio_get_level(PIN_KEY)){
					stage ++;
					count = 0;
					
				}
				break;
			case 1:
				count ++;
				if(count > 2){
					if(1 == gpio_get_level(PIN_KEY)){
						printf("the key has been push\n");
						keyPush();
						count = 0;
						stage ++;
					}
					else{
						count = 0;
						stage =0;
					}
				}
				break;
			case 2:
				if(0 == gpio_get_level(PIN_KEY)){
					stage ++;
					count = 0;
				}
				break;
			case 3:
				count ++;
				if(count > 2){
					if(0 == gpio_get_level(PIN_KEY)){
						printf("the key has been relese\n");
						count = 0;
						stage =0;
					}
					else{
						count = 0;
						stage =2;
					}
				}
				break;
			default:
				break;
		}
		vTaskDelay(10 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

void rtc_task(void *pvParameter){
	DATE_STU Time;
	char *cpTime[100];
	xEventGroupWaitBits(download_event_group, RTC_INIT_BIT,
                        false, true, portMAX_DELAY);
	xEventGroupWaitBits(download_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
	//xEventGroupSetBits(download_event_group, TIME_SYNC_BIT);
	while(1){
		//xEventGroupWaitBits(download_event_group, TIME_SYNC_BIT,
        //                false, true, portMAX_DELAY);
		sntp_sync();
		//getCurTime(&Time);
		//printf("time:%d-%d-%d %d:%d:%d\n", Time.year, Time.month, Time.day, Time.hour, Time.min, Time.sec);
		//sprintf(cpTime, "%d-%d-%d %d:%d:%d\n", Time.year, Time.month, Time.day, Time.hour, Time.min, Time.sec);
		//LCD_drawString(cpTime, 1);
		//xEventGroupClearBits(download_event_group, TIME_SYNC_BIT);
		vTaskDelay(1800*1000 / portTICK_RATE_MS); //1000s
	}
	
	vTaskDelete(NULL);
}


void LCD_test_task(void *pvParameter){
	//invertDisplay(true); //color invert...
	fillScreen(LCD_BG_COLOR);
	fillRect(5, 58, 230, 230, LCD_WORK_AREA_COLOR);
	fillRect(5, LCD_LOCATION_LOG-12, 230, 2, LCD_BG_COLOR);
	fillRect(5, LCD_LOCATION_MSG_1-7, 230, 2, LCD_BG_COLOR);
	fillRect(5,5,48,48,COLOR_WHITE);

	setTextbgColor(COLOR_RED,LCD_BG_COLOR);
	drawString("ESPRESSIF", 68, 15, 4);
	setTextbgColor(LCD_FONT_COLOR,LCD_WORK_AREA_COLOR);
	LCD_drawLOGO(5,5);
	
	LCD_drawCentreString("now syncing", LCD_LOCATION_LOG, 2, COLOR_WHITE, COLOR_BLACK);
	LCD_drawString("Ver: 1.0.0", LCD_LOCATION_MSG_1, 2, COLOR_WHITE, COLOR_BLACK);
	LCD_drawString("Bacth NO.: c0392", LCD_LOCATION_MSG_2, 2, COLOR_WHITE, COLOR_BLACK);
	LCD_drawString("Tool Ver: 1.0.0", LCD_LOCATION_MSG_3, 2, COLOR_WHITE, COLOR_BLACK);
	LCD_drawString("FlashID: GD", LCD_LOCATION_MSG_2, 4, COLOR_WHITE, COLOR_BLACK);
	LCD_drawString("device type: ESP32", LCD_LOCATION_MSG_5, 2, COLOR_WHITE, COLOR_BLACK);
	LCD_drawString("MAC: 00:00:00:00:00:00", LCD_LOCATION_MSG_6, 2, COLOR_WHITE, COLOR_BLACK);
	
	
	LCD_stageChange(0, false, 0);
	vTaskDelay(2000 / portTICK_RATE_MS);
	LCD_stageChange(1, false, 0);
	vTaskDelay(2000 / portTICK_RATE_MS);
	LCD_stageChange(2, false, 0);
	vTaskDelay(2000 / portTICK_RATE_MS);
	LCD_stageChange(3, false, 0);
	vTaskDelay(2000 / portTICK_RATE_MS);
	LCD_stageChange(3, true, 0);
	
	//LCD_drawCentreString("www.espressif.com", LCD_LOCATION_WEB, 4, LCD_BG_COLOR, COLOR_BLACK);
	while(1){
		
		vTaskDelay(10 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}


void app_main()
{
	printf("***esp_download_solution start***\n");
	printf("free heap size:%d\n", system_get_free_heap_size());
	
	download_event_group = xEventGroupCreate();
	if(esd_sys_init() != SUCCESS){printf("system init failed\n");exit(0);}
	xEventGroupWaitBits(download_event_group, CONNECTED_BIT,
                        false, true, WIFI_CONN_TIMEOUT/portTICK_RATE_MS);

	//xTaskCreate(&LCD_test_task, "LCD_test_task", 2048, NULL, 4, NULL);
	
   	xTaskCreate(&eds_task, "eds_task", 1024*40, NULL, 5, NULL);
    xTaskCreate(&key_task, "key_task", 2048, NULL, 6, NULL);
	xTaskCreate(&rtc_task, "rtc_task", 1024*8, NULL, 4, NULL);
	printf("free heap size:%d\n", system_get_free_heap_size());
	
	//xTaskCreate();
}
