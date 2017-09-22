#ifndef __LCD_API_H__
#define __LCD_API_H__
#include "eds_config.h"
#include "driver/spi_master.h"
#include "spi_lcd.h"


#define LCD_TFTWIDTH  240
#define LCD_TFTHEIGHT 320

#define LCD_LOCATION_LOGO		5
#define LCD_LOCATION_WORK_STAGE	80
#define LCD_LOCATION_LOG		140
#define LCD_LOCATION_MSG_1		170
#define LCD_LOCATION_MSG_2		190
#define LCD_LOCATION_MSG_3		210
#define LCD_LOCATION_MSG_4		230
#define LCD_LOCATION_MSG_5		250
#define LCD_LOCATION_MSG_6		270

#define LCD_LOCATION_WEB		285

#define LCD_FONT_STAGE			4
#define LCD_BG_COLOR			0xF7BB
#define LCD_WORK_AREA_COLOR		COLOR_WHITE
#define LCD_FONT_COLOR			COLOR_BLACK

/**
 * @brief draw circumference
 * @param x0 CentreX
 * @param y0 centreY
 * @param r radius
 * @param color border color
 */
void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/**
 * @brief used for drawing circles & roundRect
 * @param x0 position X
 * @param y0 position Y
 * @param r radius of round edge Ideally 5

 * @param cornername One out of 4 corners
 * @param color
 */
void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);

/**
 * @brief draw filled circle
 * @param x0 CentreX
 * @param y0 centreY
 * @param r radius
 * @param color object color
 */
void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/**
 * @brief print single char
 * @param c single character
 * @param poX position X
 * @param poY position Y
 * @param size Font size
 */
void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);

/**
 * @brief draw triangle boundary
 * @param x corners
 * @param y corners
 * @param color border color
 */
void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

/**
 * @brief draw filled triangle
 * @param x corners
 * @param y corners
 * @param color object color
 */
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

/**
 * @brief draw rounded rectangle borders
 * @param x0 position X
 * @param y0 position Y
 * @param w width
 * @param h height
 * @param r radius of edges: ideally 5
 * @param color color of border
 */
void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);

/**
 * @brief draw filled rounded rectangle
 * @param x0 position X
 * @param y0 position Y
 * @param w width
 * @param h height
 * @param r radius of edges: ideally 5
 * @param color object color
 */
void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);

/**
 * @brief Print an array of pixels: Used to display pictures usually
 * @param x position X
 * @param y position Y
 * @param bitmap pointer to bmp array

 * @param w width of image in bmp array
 * @param h height of image in bmp array
 */
void drawBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h);

/**
 * @brief Set font color, keeping background color same
 * @param c Font color
 */
void setTextColor(uint16_t c);

/**
 * @brief Set font color
 * @param c Font color
 * @param bg Font's background color
 */
void setTextbgColor(uint16_t c, uint16_t bg);

/**
 * @brief yet to figure this one out
 */
void setTextWrap(bool w);

/**
 * @brief set orientation of screen
 * @param r Enter 0 to 3, landscape/ portrait
 */
void setRotation(uint8_t r);

/*NOTE: For all the functions below, user must set Font Color first*/

/**
 * @brief print single char
 * @param c single character
 * @param poX position X
 * @param poY position Y
 * @param size Font size
 */
int drawUnicode(uint16_t uniCode, uint16_t x, uint16_t y, uint8_t size);

/**
 * @brief print whole numbers
 * @param long_num value of the number to be displayed
 * @param poX position X
 * @param poY position Y
 * @param size Font size
 */
int drawNumber(int long_num, uint16_t poX, uint16_t poY, uint8_t size);

/**
 * @brief print single char
 * @param c single character
 * @param poX position X
 * @param poY position Y
 * @param size Font size
 */
int drawChar(char c, uint16_t poX, uint16_t poY, uint8_t size);

/**
 * @brief print entire string (left aligned)
 * @param string Pointer to string
 * @param poX position X
 * @param poY position Y
 * @param size Font size
 */
int drawString(const char *string, uint16_t poX, uint16_t poY, uint8_t size);

/**
 * @brief print entire string (centre aligned)
 * @param string Pointer to string
 * @param poX position X
 * @param poY position Y
 * @param size Font size
 */
int drawCentreString(const char *string, uint16_t dX, uint16_t poY, uint8_t size);

/**
 * @brief print entire string (right aligned)
 * @param string Pointer to string
 * @param poX position X
 * @param poY position Y
 * @param size Font size
 */
int drawRightString(const char *string, uint16_t dX, uint16_t poY, uint8_t size);

/**
 * @brief print Floating numbers
 * @param floatNumber Float value of the number to be displayed
 * @param decimal Places after the decimal point
 * @param x position X
 * @param y position Y
 * @param size Font size
 */
int drawFloat(float floatNumber, uint8_t decimal, uint16_t poX, uint16_t poY, uint8_t size);

/*Gives the current height acc. to rotation*/
int16_t height(void);

/*Gives the current width acc. to rotation*/
int16_t width(void);

/*Gives the current orientation state*/
uint8_t getRotation(void);

typedef struct _SCREEN{
	const int16_t WIDTH;
    const int16_t HEIGHT;   // This is the 'raw' display w/h - never changes
    int16_t _width;
    int16_t _height;        // Display w/h as modified by current rotation
    uint16_t textcolor;
    uint16_t textbgcolor;
    uint8_t rotation;
    bool wrap;              // If set, 'wrap' text at right edge of display
}TFT;


void lcd_api_init(lcd_pin_conf_t *lcd_pins);
void LCD_drawCentreString(const char *string, uint16_t local, uint8_t font_size, uint16_t bg_color, uint16_t font_color);
void LCD_stageChange(uint8_t module_stage, bool load_result, uint8_t tool_no);
void LCD_drawLOGO(uint16_t x, uint16_t y);
void LCD_drawString(const char *string, uint16_t local, uint8_t font_size, uint16_t bg_color, uint16_t font_color);

#endif //__LCD_API_H__

