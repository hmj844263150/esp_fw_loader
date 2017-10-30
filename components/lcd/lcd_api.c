#include "lcd_api.h"
#include "lcd_func.h"
#include "spi_lcd.h"
#include <stdlib.h> 
#include "Load_fonts.h"
#include "Font16.h"
#include "Font32.h"
#include "Font64.h"
#include "Font7s.h"


#define swap(x,y)	(x=(x)+(y), y=(x)-(y), x=(x)-(y))
extern spi_device_handle_t spi;	


TFT tft = {.WIDTH = LCD_TFTWIDTH, .HEIGHT = LCD_TFTHEIGHT};

void Screen_init(TFT _tft){
	tft._width = _tft._width;
	tft._height = _tft._height;
	tft.rotation = _tft.rotation;
	tft.textcolor = _tft.textcolor;
	tft.textbgcolor = _tft.textbgcolor;
}

const unsigned char ESP_LOGO_40x40[] = {	// 6 byte per row
	0xff,	0xff,	0xff,	0xff,	0xff,	0xff, 
	0xff,	0xff,	0xff,	0xff,	0xff,	0xff,
	0xff,	0xff,	0xff,	0xff,	0xff,	0xff, 
	0xff,	0xff,	0xff,	0xff,	0xff,	0xff,
	0xff,	0xff,	0xff,	0xff,	0xff,	0xff,
	0xff,	0xff,	0xff,	0x1f,	0xff,	0xff,
	0xff,	0xff,	0xfe,	0x01,	0xff,	0xff,
	0xff,	0xff,	0x3f,	0x00,	0x3f,	0xff,
	0xff,	0x9c,	0x03,	0xe0,	0x1f,	0xff,
	0xff,	0x98,	0x00,	0x78,	0x0f,	0xff,
	0xff,	0x38,	0x00,	0x1e,	0x07,	0xff,
	0xfe,	0x78,	0x00,	0x0f,	0x03,	0xff,
	0xfc,	0xf8,	0x00,	0x03,	0x01,	0xff,
	0xfc,	0xff,	0xf0,	0x01,	0xc0,	0xff,
	0xf8,	0xff,	0xfc,	0x00,	0xe0,	0x7f,
	0xf9,	0xe0,	0x7f,	0x00,	0x60,	0x7f,
	0xf9,	0x80,	0x1f,	0xc0,	0x78,	0x3f,
	0xf3,	0x00,	0x03,	0xe0,	0x1c,	0x3f,
	0xf3,	0x00,	0x01,	0xf0,	0x1c,	0x3f,
	0xf2,	0x00,	0x00,	0x78,	0x1e,	0x1f,
	0xf2,	0x00,	0x00,	0x3c,	0x06,	0x1f,
	0xf2,	0x07,	0xe0,	0x1e,	0x07,	0x1f,
	0xf6,	0x07,	0xf8,	0x0f,	0x03,	0x1f,
	0xf6,	0x00,	0xfc,	0x07,	0x83,	0x1f,
	0xf2,	0x00,	0x3e,	0x03,	0x83,	0x1f,
	0xf3,	0x00,	0x0f,	0x03,	0xc1,	0xff,
	0xf3,	0x80,	0x07,	0x81,	0xc1,	0xff,
	0xf3,	0xc0,	0x03,	0xc1,	0xc1,	0xff,
	0xf3,	0xe0,	0x01,	0xc1,	0xe0,	0xff,
	0xfb,	0xff,	0x01,	0xe0,	0xe0,	0xff,
	0xf9,	0xff,	0xc0,	0xe0,	0xe0,	0xff,
	0xf9,	0xfb,	0xe0,	0xe0,	0xe0,	0xff,
	0xfc,	0xf1,	0xe0,	0xe0,	0xe0,	0xff,
	0xfc,	0xe0,	0xf0,	0xf0,	0xe1,	0xff,
	0xfe,	0xe0,	0xf0,	0xf0,	0xe3,	0xff,
	0xfe,	0xe0,	0xf0,	0xf0,	0xff,	0xff,
	0xff,	0x20,	0xe0,	0xf0,	0xff,	0xff,
	0xff,	0x9b,	0xe0,	0xe0,	0xfd,	0xff,
	0xff,	0xcf,	0xc0,	0xe0,	0xf8,	0xff,
	0xff,	0xe3,	0xe0,	0xe1,	0xf3,	0xff,
	0xff,	0xf0,	0xf9,	0xf3,	0xc7,	0xff,
	0xff,	0xfc,	0x3f,	0xff,	0x0f,	0xff,
	0xff,	0xfe,	0x07,	0xf0,	0x7f,	0xff,
	0xff,	0xff,	0xe0,	0x03,	0xff,	0xff,
	0xff,	0xff,	0xff,	0x3f,	0xff,	0xff,
	0xff,	0xff,	0xff,	0xff,	0xff,	0xff,
	0xff,	0xff,	0xff,	0xff,	0xff,	0xff,
	0xff,	0xff,	0xff,	0xff,	0xff,	0xff
};


// Draw a circle outline
void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            drawPixel(x0 - y, y0 - x, color);
            drawPixel(x0 - x, y0 - y, color);
        }
    }
}

void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            drawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            drawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
        if (cornername & 0x2) {
            drawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            drawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}

// Bresenham's algorithm - thx wikpedia
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}
/*
void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // Update in subclasses if desired!
    drawLine(x, y, x, y + h - 1, color);
}

void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // Update in subclasses if desired!
    drawLine(x, y, x + w - 1, y, color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // Update in subclasses if desired!
    for (int16_t i = x; i < x + w; i++) {
        drawFastVLine(i, y, h, color);
    }
}

void fillScreen(uint16_t color)
{
    fillRect(0, 0, tft._width,tft._height, color);
}
*/
void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
    // smarter version
    drawFastHLine(x + r  , y    , w - 2 * r, color); // Top
    drawFastHLine(x + r  , y + h - 1, w - 2 * r, color); // Bottom
    drawFastVLine(x    , y + r  , h - 2 * r, color); // Left
    drawFastVLine(x + w - 1, y + r  , h - 2 * r, color); // Right
    // draw four corners
    drawCircleHelper(x + r    , y + r    , r, 1, color);
    drawCircleHelper(x + w - r - 1, y + r    , r, 2, color);
    drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    drawCircleHelper(x + r    , y + h - r - 1, r, 8, color);
}

void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
    // smarter version
    fillRect(x + r, y, w - 2 * r, h, color);
    // draw four corners
    fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    fillCircleHelper(x + r    , y + r, r, 2, h - 2 * r - 1, color);
}

void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

void fillTriangle (int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    int16_t a, b, y, last;
    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        swap(y0, y1); swap(x0, x1);
    }
    if (y1 > y2) {
        swap(y2, y1); swap(x2, x1);
    }
    if (y0 > y1) {
        swap(y0, y1); swap(x0, x1);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a) {
            a = x1;
        } else if (x1 > b) {
            b = x1;
        }
        if (x2 < a) {
            a = x2;
        } else if (x2 > b) {
            b = x2;
        }
        drawFastHLine(a, y0, b - a + 1, color);
        return;
    }
    int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa   = 0,
    sb   = 0;

    /*For upper part of triangle, find scanline crossings for segments
    0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    is included here (and second loop will be skipped, avoiding a /0
    error there), otherwise scanline y1 is skipped here and handled
    in the second loop...which also avoids a /0 error here if y0=y1
    (flat-topped triangle)*/
    if (y1 == y2) {
        last = y1;   // Include y1 scanline
    } else {
        last = y1 - 1; // Skip it
    }

    for (y = y0; y <= last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b) {
            swap(a, b);
        }
        drawFastHLine(a, y, b - a + 1, color);
    }

    /* For lower part of triangle, find scanline crossings for segments
    0-2 and 1-2.  This loop is skipped if y1=y2*/
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b) {
            swap(a, b);
        }
        drawFastHLine(a, y, b - a + 1, color);
    }
}
/*
void drawBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h)
{
    for (uint16_t j = 0; j < h; j++) {
        for (uint16_t i = 0; i < w; i++) {
            drawPixel(x + i, y + j, bitmap[j * w + i]);
        }
    }
}
*/
void setTextColor(uint16_t c)
{
    /* For 'transparent' background, we'll set the bg
    to the same as fg instead of using a flag*/
    tft.textcolor = tft.textbgcolor = c;
}

void setTextbgColor(uint16_t c, uint16_t b)
{
    tft.textcolor   = c;
    tft.textbgcolor = b;
}

uint8_t getRotation(void)
{
    return tft.rotation;
}

void setRotation(uint8_t x)
{
    tft.rotation = (x % 4);
    /*switch (tft.rotation) {
    case 0:
    case 2: 
		tft._width  = tft.WIDTH;
        tft._height = tft.HEIGHT;
        break;
    
    case 1:
    case 3:
		tft._width  = tft.HEIGHT;
        tft._height = tft.WIDTH;
        break;
    
    }*/
    switch (tft.rotation) {
    case 0:
    case 2: 
		tft._width  = 240;
        tft._height = 320;
        break;
    
    case 1:
    case 3:
		tft._width  = 320;
        tft._height = 240;
        break;
    
    }
	_setRotation(tft.rotation);
}

int16_t width(void)
{
    return tft._width;
}

int16_t height(void)
{
    return tft._height;
}

static void drawDoubPixel(int16_t x,int16_t y,uint16_t color){
	for(int i=0; i<2; i++)
		for(int j=0;j<2;j++)
			drawPixel(x+i, y+j, color);
}

int drawUnicode(uint16_t uniCode, uint16_t x, uint16_t y, uint8_t size)
{
    if (size) {
        uniCode -= 32;
    }
    uint16_t width = 0;
    uint16_t height = 0;
    const uint8_t *flash_address = 0;
    int8_t gap = 0;

#ifdef LOAD_FONT2
    if (size == 2) {
        flash_address = chrtbl_f16[uniCode];
        width = *(widtbl_f16 + uniCode);
        height = chr_hgt_f16;
        gap = 1;
    }
#endif

#ifdef LOAD_FONT4
    if (size == 4) {
        flash_address = chrtbl_f32[uniCode];
        width = *(widtbl_f32 + uniCode);
        height = chr_hgt_f32;
        gap = -3;
    }
	if (size == 8) {
		flash_address = chrtbl_f32[uniCode];
		width = *(widtbl_f32 + uniCode);
        height = chr_hgt_f32;
        gap = -3;
	}
#endif

#ifdef LOAD_FONT6
    if (size == 6) {
        flash_address = chrtbl_f64[uniCode];
        width = *(widtbl_f64 + uniCode);
        height = chr_hgt_f64;
        gap = -3;
    }
#endif

#ifdef LOAD_FONT7
    if (size == 7) {
        flash_address = chrtbl_f7s[uniCode];
        width = *(widtbl_f7s + uniCode);
        height = chr_hgt_f7s;
        gap = 2;
    }
#endif

    uint16_t w = (width + 7) / 8;
    uint16_t pX      = 0;
    uint16_t pY      = y;
    uint8_t line = 0;
    for (int i = 0; i < height; i++) {
		
        if (tft.textcolor != tft.textbgcolor) {
            drawFastHLine(x, pY, width + gap, tft.textbgcolor);
        }
		
        for (int k = 0; k < w; k++)      {
            line = *(flash_address + w*i+k);
            if (line) {
                pX = x + k * 8;
                if (line & 0x80) {
					if(size == 8)
						drawDoubPixel(x+(pX-x)*2, y+(pY-y)*2, tft.textcolor);
					else
                    	drawPixel(pX, pY, tft.textcolor);
                }
                if (line & 0x40) {
					if(size == 8)
						drawDoubPixel(x+(pX+1-x)*2, y+(pY-y)*2, tft.textcolor);
					else
	                    drawPixel(pX + 1, pY, tft.textcolor);
                }
                if (line & 0x20) {
					if(size == 8)
						drawDoubPixel(x+(pX+2-x)*2, y+(pY-y)*2, tft.textcolor);
					else
	                    drawPixel(pX + 2, pY, tft.textcolor);
                }
                if (line & 0x10) {
					if(size == 8)
						drawDoubPixel(x+(pX+3-x)*2, y+(pY-y)*2, tft.textcolor);
					else
	                    drawPixel(pX + 3, pY, tft.textcolor);
                }
                if (line & 0x8) {
					if(size == 8)
						drawDoubPixel(x+(pX+4-x)*2, y+(pY-y)*2, tft.textcolor);
					else
                    	drawPixel(pX + 4, pY, tft.textcolor);
                }
                if (line & 0x4) {
					if(size == 8)
						drawDoubPixel(x+(pX+5-x)*2, y+(pY-y)*2, tft.textcolor);
					else
                    	drawPixel(pX + 5, pY, tft.textcolor);
                }
                if (line & 0x2) {
					if(size == 8)
						drawDoubPixel(x+(pX+6-x)*2, y+(pY-y)*2, tft.textcolor);
					else
                    	drawPixel(pX + 6, pY, tft.textcolor);
                }
                if (line & 0x1) {
					if(size == 8)
						drawDoubPixel(x+(pX+7-x)*2, y+(pY-y)*2, tft.textcolor);
					else
                    	drawPixel(pX + 7, pY, tft.textcolor);
                }
            }
        }
        pY++;
    }
	if(size == 8)
		return (width + gap) * 2;
    return width + gap;      // x +
}

int drawNumber(int long_num, uint16_t poX, uint16_t poY, uint8_t size)
{
    char tmp[10];
    if (long_num < 0) {
        snprintf(tmp, sizeof(tmp), "%d", long_num);
    } else {
        snprintf(tmp, sizeof(tmp), "%u", long_num);
    }
    return drawString(tmp, poX, poY, size);
}

int drawChar(char c, uint16_t poX, uint16_t poY, uint8_t size)
{
    return drawUnicode(c, poX, poY, size);
}

int drawString(const char *string, uint16_t poX, uint16_t poY, uint8_t size)
{
    uint16_t sumX = 0;
    while (*string) {
        uint16_t xPlus = drawChar(*string, poX, poY, size);
        sumX += xPlus;
        string++;
        poX += xPlus;                                     /* Move cursor right*/
    }
    return sumX;
}

int drawCentreString(const char *string, uint16_t dX, uint16_t poY, uint8_t size)
{
    uint16_t sumX = 0;
    uint16_t len = 0;
    const char *pointer = string;
    char ascii;

    while (*pointer)    {
        ascii = *pointer;
#ifdef LOAD_FONT2
        if (size == 2) {
            len += 1 + *(widtbl_f16 + ascii - 32);
        }
#endif
#ifdef LOAD_FONT4
        if (size == 4) {
            len += *(widtbl_f32 + ascii - 32) - 3;
        }
		if (size == 8) {
			len += (*(widtbl_f32 + ascii - 32) - 3)*2;
		}
#endif
#ifdef LOAD_FONT6
        if (size == 6) {
            len += *(widtbl_f64 + ascii - 32) - 3;
        }
#endif
#ifdef LOAD_FONT7
        if (size == 7) {
            len += *(widtbl_f7s + ascii - 32) + 2;
        }
        pointer++;
#endif
    }
    int poX = (dX - len) / 2;
    if (poX < 0) {
        poX = 0;
    }
    while (*string) {
        uint16_t xPlus = drawChar(*string, poX, poY, size);
        sumX += xPlus;
        string++;
        if (poX < 264) {
            poX += xPlus;                                     /* Move cursor right */
        }
    }
    return sumX;
}

int drawRightString(const char *string, uint16_t dX, uint16_t poY, uint8_t size)
{
    int sumX = 0;
    int len = 0;
    const char *pointer = string;
    char ascii;

    while (*pointer)    {
        ascii = *pointer;
#ifdef LOAD_FONT2
        if (size == 2) {
            len += 1 + *(widtbl_f16 + ascii - 32);
        }
#endif
#ifdef LOAD_FONT4
        if (size == 4) {
            len += *(widtbl_f32 + ascii - 32) - 3;
        }
#endif
#ifdef LOAD_FONT6
        if (size == 6) {
            len += *(widtbl_f64 + ascii - 32) - 3;
        }
#endif
#ifdef LOAD_FONT7
        if (size == 7) {
            len += *(widtbl_f7s + ascii - 32) + 2;
        }
#endif
        pointer++;
    }

    int poX = dX - len;
    if (poX < 0) {
        poX = 0;
    }

    while (*string) {
        uint16_t xPlus = drawChar(*string, poX, poY, size);
        sumX += xPlus;
        string++;
        if (poX < 264) {
            poX += xPlus;                                     /* Move cursor right */
        }
    }
    return sumX;
}

int drawFloat(float floatNumber, uint8_t decimal, uint16_t poX, uint16_t poY, uint8_t size)
{
    unsigned int temp = 0;
    float decy = 0.0;
    float rounding = 0.5;
    float eep = 0.000001;
    int sumX = 0;
    uint16_t xPlus = 0;

    if (floatNumber - 0.0 < eep) {
        xPlus = drawChar('-', poX, poY, size);
        floatNumber = -floatNumber;
        poX  += xPlus;
        sumX += xPlus;
    }

    for (unsigned char i = 0; i < decimal; ++i) {
        rounding /= 10.0;
    }
    floatNumber += rounding;
    temp = (long)floatNumber;
    xPlus = drawNumber(temp, poX, poY, size);
    poX  += xPlus;
    sumX += xPlus;

    if (decimal > 0) {
        xPlus = drawChar('.', poX, poY, size);
        poX += xPlus;                                       /* Move cursor right   */
        sumX += xPlus;
    } else {
        return sumX;
    }

    decy = floatNumber - temp;
    for (unsigned char i = 0; i < decimal; i++) {
        decy *= 10;                                         /* For the next decimal*/
        temp = decy;                                        /* Get the decimal     */
        xPlus = drawNumber(temp, poX, poY, size);
        poX += xPlus;                                       /* Move cursor right   */
        sumX += xPlus;
        decy -= temp;
    }
    return sumX;
}


void lcd_api_init(lcd_pin_conf_t *lcd_pins){
	/*
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
	*/

    /*Initialize SPI Handler*/
    lcd_init(lcd_pins, &spi);
	TFT _tft={._width=LCD_TFTWIDTH, ._height=LCD_TFTHEIGHT, .rotation=1, .textcolor=0xffff, .textbgcolor=0x0};
	Screen_init(_tft);
	fillScreen(0x0);
}

void LCD_drawCentreString(const char *string, uint16_t local, uint8_t font_size, uint16_t bg_color, uint16_t font_color){
	fillRect(5,local,230,8*font_size>64?64:8*font_size,bg_color); 
	setTextbgColor(font_color, bg_color);
	drawCentreString(string, 240, local, font_size);
	setTextbgColor(LCD_FONT_COLOR,LCD_WORK_AREA_COLOR);
}

void LCD_drawString(const char *string, uint16_t local, uint8_t font_size, uint16_t bg_color, uint16_t font_color){
	fillRect(5,local,230,8*font_size>64?64:8*font_size,bg_color); 
	setTextbgColor(font_color, bg_color);
	drawString(string, 10, local, font_size);
}


void LCD_drawLOGO(uint16_t x, uint16_t y){
	uint16_t w = 6;
    uint16_t pX      = 0;
    uint16_t pY      = y;
    uint8_t line = 0;
	int height = 48;
	setTextbgColor(COLOR_RED, COLOR_WHITE);
    for (int i = 0; i < height; i++) {
		/*
        if (tft.textcolor != tft.textbgcolor) {
            drawFastHLine(x, pY, width, tft.textbgcolor);
        }
		*/
        for (int k = 0; k < w; k++)      {
            line = *(ESP_LOGO_40x40 + w*i+k);
            if (line) {
                pX = x + k * 8;
                if (line & 0x80) {
                    drawPixel(pX, pY, tft.textcolor);
                }
                if (line & 0x40) {
                    drawPixel(pX + 1, pY, tft.textcolor);
                }
                if (line & 0x20) {
                    drawPixel(pX + 2, pY, tft.textcolor);
                }
                if (line & 0x10) {
                    drawPixel(pX + 3, pY, tft.textcolor);
                }
                if (line & 0x8) {
                    drawPixel(pX + 4, pY, tft.textcolor);
                }
                if (line & 0x4) {
                    drawPixel(pX + 5, pY, tft.textcolor);
                }
                if (line & 0x2) {
                    drawPixel(pX + 6, pY, tft.textcolor);
                }
                if (line & 0x1) {
                    drawPixel(pX + 7, pY, tft.textcolor);
                }
            }
        }
        pY++;
    }
	setTextbgColor(LCD_FONT_COLOR,LCD_WORK_AREA_COLOR);
}

void LCD_stageChange(uint8_t module_stage, bool load_result, uint8_t tool_no){
	char prtStr[100];
	switch(module_stage){
		case MS_DRIVER_INIT:
			fillRect(5,58,230,72,COLOR_BLUE); 
			LCD_drawCentreString("sys starting", LCD_LOCATION_WORK_STAGE, 4, COLOR_BLUE, COLOR_WHITE);
			break;
		case MS_WAIT_MODULE:
			fillRect(5,58,230,72,COLOR_BLUE); 
			LCD_drawCentreString("IDLE", LCD_LOCATION_WORK_STAGE, 4, COLOR_BLUE, COLOR_WHITE);
			break;
		case MS_FW_DOWNLOAD:
			fillRect(5,58,230,72,COLOR_YELLOW); 
			LCD_drawCentreString("Downloading...", LCD_LOCATION_WORK_STAGE, 4, COLOR_YELLOW, COLOR_RED);
			break;
		case MS_MDL_TEST:
			fillRect(5,58,230,72,COLOR_YELLOW); 
			LCD_drawCentreString("Testing...", LCD_LOCATION_WORK_STAGE, 4, COLOR_YELLOW, COLOR_RED);
			break;
		case MS_LOAD_RAM:
			fillRect(5,58,230,72,COLOR_YELLOW); 
			LCD_drawCentreString("Load to Ram", LCD_LOCATION_WORK_STAGE, 4, COLOR_YELLOW, COLOR_RED);
			break;
		case MS_FW_CHECK:
			if(load_result){
				sprintf(prtStr, "%d-PASS", tool_no);
				fillRect(5,58,230,72,COLOR_GREEN); 
				LCD_drawCentreString(prtStr, LCD_LOCATION_WORK_STAGE, 4, COLOR_GREEN, COLOR_RED);
			}
			else{
				sprintf(prtStr, "%d-FAIL", tool_no);
				fillRect(5,58,230,72,COLOR_RED); 
				LCD_drawCentreString("FAIL", LCD_LOCATION_WORK_STAGE, 4, COLOR_RED, COLOR_YELLOW);
			}
			break;
	}
}


