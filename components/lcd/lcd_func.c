#include "lcd_func.h"
#include <malloc.h>
#include "spi_lcd.h"
#include "lcd_api.h"

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

#define SWAPBYTES(i) ((i>>8) | (i<<8))

extern TFT tft;
extern spi_device_handle_t spi;

void transmitCmdData(uint8_t cmd, uint32_t data)
{
    lcd_cmd(spi, cmd);
    lcd_data(spi, (uint8_t *)&data, 4);
}

void transmitData_uint16_r(uint16_t data, int32_t repeats)
{
    lcd_send_uint16_r(spi, data, repeats);
}

void transmitCmd(uint8_t cmd)
{
    lcd_cmd(spi, cmd);
}

void transmitData(uint8_t* data, int length)
{
    lcd_data(spi, (uint8_t *)data, length);
}


static void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    transmitCmdData(LCD_CASET, MAKEWORD(x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF));
    transmitCmdData(LCD_PASET, MAKEWORD(y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF));
    transmitCmd(LCD_RAMWR); // write to RAM
}

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= tft._width) || (y < 0) || (y >= tft._height)) {
        return;
    }
    setAddrWindow(x, y, x + 1, y + 1);
	color = SWAPBYTES(color);
    transmitData((uint8_t*)&color, 2);
}

void drawBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h)
{
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    int point_num = w * h ;
    uint16_t* data_buf = (uint16_t*) malloc(point_num * sizeof(uint16_t));
    for (int i = 0; i < point_num; i++) {
        data_buf[i] = SWAPBYTES(bitmap[i]);
    }

    int gap_point = 1024;
    uint16_t* cur_ptr = data_buf;
    while(point_num > 0) {
        int trans_points = point_num > gap_point ? gap_point : point_num;
        transmitData((uint8_t*)(cur_ptr), sizeof(uint16_t) * trans_points);
        cur_ptr += trans_points;
        point_num -= trans_points;
    }
    free(data_buf);
    data_buf = NULL;
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // Rudimentary clipping
    if ((x >= tft._width) || (y >= tft._height)) {
        return;
    }
    if ((y + h - 1) >= tft._height) {
        h = tft._height - y;
    }
    setAddrWindow(x, y, x, y + h - 1);
    transmitData_uint16_r(SWAPBYTES(color), h);
}

void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // Rudimentary clipping
    if ((x >= tft._width) || (y >= tft._height)) {
        return;
    }
    if ((x + w - 1) >= tft._width) {
        w = tft._width - x;
    }
    setAddrWindow(x, y, x + w - 1, y);
    transmitData_uint16_r(SWAPBYTES(color), w);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // rudimentary clipping (drawChar w/big text requires this)
    if ((x >= tft._width) || (y >= tft._height)) {
        return;
    }
    if ((x + w - 1) >= tft._width) {
        w = tft._width  - x;
    }
    if ((y + h - 1) >= tft._height) {
        h = tft._height - y;
    }
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    transmitData_uint16_r(SWAPBYTES(color), h * w);
}

void fillScreen(uint16_t color)
{
    fillRect(0, 0, tft._width, tft._height, color);
}

void scrollTo(uint16_t y)
{
    transmitCmd(0x37);
    transmitData((uint8_t*)&y, 2);
}

void _setRotation(uint8_t m)
{
    uint8_t data = 0;
    tft.rotation = m % 4;  //Can't be more than 3
    switch (tft.rotation) {
    case 0:
        data = MADCTL_MX | MADCTL_BGR;
        tft._width  = LCD_TFTWIDTH;
        tft._height = LCD_TFTHEIGHT;
        break;
    case 1:
        data = MADCTL_MV | MADCTL_BGR;
        tft._width  = LCD_TFTHEIGHT;
        tft._height = LCD_TFTWIDTH;
        break;
    case 2:
        data = MADCTL_MY | MADCTL_BGR;
        tft._width  = LCD_TFTWIDTH;
        tft._height = LCD_TFTHEIGHT;
        break;
    case 3:
        data = MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR;
        tft._width  = LCD_TFTHEIGHT;
        tft._height = LCD_TFTWIDTH;
        break;
    }
	
    transmitCmdData(LCD_MADCTL, data);
}

void spicial_set(void){
	uint8_t data = MADCTL_MX | MADCTL_MV | MADCTL_ML | MADCTL_RGB;
    tft._width  = LCD_TFTWIDTH;
    tft._height = LCD_TFTHEIGHT;
	transmitCmdData(LCD_MADCTL, data);
}


void invertDisplay(bool i)
{
    transmitCmd( i ? LCD_INVON : LCD_INVOFF);
}



