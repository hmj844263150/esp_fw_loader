#include "eds_driver_init.h"

#include <string.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"

#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "spi_lcd.h"
#include "lcd_api.h"
#include "lcd_func.h"

#include "soc/soc.h"
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"


static const char* TAG = "driver init";

//#define ASSERT_CHECK(x)	err_no=x;if(err_no!=SUCCESS){ printf("get a err no:%d\n", err_no);goto ERR_OVER;}

static int gpio_driver_init(void){
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO16/17
    io_conf.pin_bit_mask = (1<<ESP_CHIP_EN) | (1<<ESP_GPIO_0);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	
	return 0;
}

static int uart_init(void){
	const int uart_num = UART_NUM_1;
	esp_err_t errno = 0;
	uart_config_t uart_config = {        
			.baud_rate = 115200,     
			.data_bits = UART_DATA_8_BITS,        
			.parity = UART_PARITY_DISABLE,        
			.stop_bits = UART_STOP_BITS_1,        
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,        
			.rx_flow_ctrl_thresh = 0,    
			};    //Configure UART1 parameters    
	errno |= uart_param_config(uart_num, &uart_config);    
	//Set UART1 pins(TX: IO4, RX: I05, RTS: IO18, CTS: IO19)    
	errno |= uart_set_pin(uart_num, UART1_TXD, UART1_RXD, UART1_RTS, UART1_CTS);    
	//Install UART driver (we don't need an event queue here)    
	//In this example we don't even use a buffer for sending data.    
	errno |= uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
	uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);
	/*
	char *test="test data";
	while(1){
		uart_write_bytes(UART_NUM_1, (const char*)test, strlen(test)+1);
		vTaskDelay(100);
	}
	*/
	if(errno != ESP_OK)
		return -1;
	return 0;
}

static int spi_master_init(void){
	return 0;
}

static int spi_lcd_init(void){
	lcd_pin_conf_t lcd_pins = {
        .lcd_model    = ILI9341,
        .pin_num_miso = PIN_NUM_MISO,
        .pin_num_mosi = PIN_NUM_MOSI,
        .pin_num_clk  = PIN_NUM_CLK,
        .pin_num_cs   = PIN_NUM_CS,
        .pin_num_dc   = PIN_NUM_DC,
        .pin_num_rst  = PIN_NUM_RST,
        .pin_num_bckl = PIN_NUM_BCKL,
    };

    /*Initialize SPI Handler*/
	
    lcd_api_init(&lcd_pins);
	setRotation(0);
	if(NEW_VERSION)
		spicial_set();
	fillScreen(0xffff);
	setTextbgColor(0x0, 0xffff);

	fillScreen(LCD_BG_COLOR);
	fillRect(5, 58, 230, 230, LCD_WORK_AREA_COLOR);
	fillRect(5, LCD_LOCATION_LOG-12, 230, 2, LCD_BG_COLOR);
	fillRect(5, LCD_LOCATION_MSG_1-7, 230, 2, LCD_BG_COLOR);
	fillRect(5,5,48,48,COLOR_WHITE);

	setTextbgColor(COLOR_RED,LCD_BG_COLOR);
	drawString("ESPRESSIF", 68, 15, 4);
	setTextbgColor(LCD_FONT_COLOR,LCD_WORK_AREA_COLOR);
	LCD_drawLOGO(5,5);
	LCD_stageChange(0, false, 0);
	return 0;
}

static int sd_init(void){

	ESP_LOGW(TAG, "Initializing SD card");

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:
    // host.flags = SDMMC_HOST_FLAG_1BIT;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and formatted
    // in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5
    };
	// Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGW(TAG, "Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGW(TAG, "Failed to initialize the card (%d). Make sure SD card lines have pull-up resistors in place.", ret);
        }
        return ERR_DRIVER;
    }
	
	return 0;
}

static int wd_init(void){
	return 0;
}

void rtc_modual_init(void)
{
	
}	


ERR_STATUS eds_driver_init(enum BOARD_TYPE board_type){
	
	int err_no=0;
	err_no |= gpio_driver_init();
	if(0==err_no) printf("gpio init fin\n");
	err_no |= uart_init();
	if(0==err_no) printf("uart init fin\n");
	
	if(board_type == BOT_WROVERKIT){
		err_no |= sd_init();
		if(0==err_no) printf("sd card init fin\n");
		vTaskDelay(100);
		err_no |= spi_lcd_init();
		if(0==err_no) printf("lcd init fin\n");
	}
	else if(board_type == BOT_8089AGING){
		spi_flash_init();
	}

	if(err_no != SUCCESS)
		return ERR_DRIVER;
	return SUCCESS;
}


