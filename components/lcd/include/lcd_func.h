#ifndef __LCD_FUNC_H__
#define __LCD_FUNC_H__
#include "eds_config.h"
#include "driver/spi_master.h"


#define LCD_INVOFF    0x20
#define LCD_INVON     0x21

#define LCD_CASET   0x2A
#define LCD_PASET   0x2B
#define LCD_RAMWR   0x2C
#define LCD_MADCTL  0x36

// Color definitions
#define COLOR_BLACK       0x0000      /*   0,   0,   0 */
#define COLOR_NAVY        0x000F      /*   0,   0, 128 */
#define COLOR_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define COLOR_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define COLOR_MAROON      0x7800      /* 128,   0,   0 */
#define COLOR_PURPLE      0x780F      /* 128,   0, 128 */
#define COLOR_OLIVE       0x7BE0      /* 128, 128,   0 */
#define COLOR_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define COLOR_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define COLOR_BLUE        0x001F      /*   0,   0, 255 */
#define COLOR_GREEN       0x07E0      /*   0, 255,   0 */
#define COLOR_CYAN        0x07FF      /*   0, 255, 255 */
#define COLOR_RED         0xF800      /* 255,   0,   0 */
#define COLOR_MAGENTA     0xF81F      /* 255,   0, 255 */
#define COLOR_YELLOW      0xFFE0      /* 255, 255,   0 */
#define COLOR_WHITE       0xFFFF      /* 255, 255, 255 */
#define COLOR_ORANGE      0xFD20      /* 255, 165,   0 */
#define COLOR_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define COLOR_PINK        0xF81F
#define COLOR_SILVER      0xC618
#define COLOR_GRAY        0x8410
#define COLOR_LIME        0x07E0
#define COLOR_TEAL        0x0410
#define COLOR_FUCHSIA     0xF81F
#define COLOR_ESP_BKGD    0xD185




#define MAKEWORD(b1, b2, b3, b4) ((uint32_t)(b1) | ((b2) << 8) | ((b3) << 16) | ((b4) << 24))

void lcd_test(void *pvParameter);
extern spi_device_handle_t spi;

void transmitCmdData(uint8_t cmd, uint32_t data);

void transmitData_uint16_r(uint16_t data, int32_t repeats);
void transmitCmd(uint8_t cmd);
void transmitData(uint8_t* data, int length);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
void drawPixel(int16_t x, int16_t y, uint16_t color);
void drawBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h);
void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fillScreen(uint16_t color);
void scrollTo(uint16_t y);
void _setRotation(uint8_t m);
void invertDisplay(bool i);
void spicial_set(void);



#endif //__LCD_FUNC_H__


