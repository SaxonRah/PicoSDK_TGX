///////////////////////////////////////////////////////////////////////////////
//
//  Modified ILI9341 driver for TGX library compatibility
//  Based on original work by Roman Piksaykin [piksaykin@gmail.com], R2BDY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _ILI9341_TGX_H
#define _ILI9341_TGX_H

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "../lib/assert.h"

#include "ili9341hw.h"

// Include TGX headers for C++
#ifdef __cplusplus
#include "../tgx/tgx.h"
using namespace tgx;
#endif

// Color definitions
typedef enum 
{
    kBlack = 0,
    kBlue,
    kRed,
    kMagenta,
    kGreen,
    kCyan,
    kYellow,
    kWhite
} color_t;

// Hardware configuration structure (unchanged)
typedef struct 
{
    spi_inst_t *mpSPIPort;
    int mGPIO_miso;
    int mGPIO_cs;
    int mGPIO_sck;
    int mGPIO_mosi;
    int mGPIO_reset;
    int mGPIO_dc;
} ili9341_config_t;

// Screen control structure with RGB565 framebuffer
typedef struct
{
    ili9341_config_t *mpHWConfig;       
    
    int16_t mCursorX;                   
    int16_t mCursorY;                   
    
    color_t mCanvasPaper;               
    color_t mCanvasInk;                 
    
    uint16_t *mpFrameBuffer;            // RGB565 framebuffer pointer
    
    #ifdef __cplusplus
    Image<RGB565>* mpTGXImage;          // TGX Image wrapper
    #else
    void* mpTGXImage;                   
    #endif
    
} screen_control_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Hardware I/O low level operations (unchanged) */
void ILI9341_Init(ili9341_config_t *pconfig, spi_inst_t *pspi_port, 
                    int spi_clock_freq, int gpio_MISO, int gpio_CS, 
                    int gpio_SCK, int gpio_MOSI, int gpio_RS, int gpio_DC);

void ILI9341_SetCommand(const ili9341_config_t *pconfig, uint8_t cmd);
void ILI9341_CommandParam(const ili9341_config_t *pconfig, uint8_t data);
void ILI9341_SetOutWriting(const ili9341_config_t *pconfig,
                            const int start_col, const int end_col,
                            const int start_page, const int end_page);
void ILI9341_WriteData(const ili9341_config_t *pconfig, void *buffer, int bytes);

/* Screen initialization */
void TftInitScreen(screen_control_t *pscr, ili9341_config_t *pconfig, 
                   uint16_t *framebuffer);

/* Buffer operations */
void TftClearScreenBuffer(screen_control_t *pscr, color_t paper, color_t ink);
void TftSetCursor(screen_control_t *pscr, int x, int y);
void TftPutChar(screen_control_t *pscr, int x, int y, color_t paper, 
                color_t ink, char chr);
void TftPutString(screen_control_t *pscr, const char* str, int top_y, 
                  int bot_y, color_t paper, color_t ink);
void TftPrintf(screen_control_t *pscr, int top_y, int bot_y, color_t paper,
               color_t ink, const char* str, ...);

/* Graphics operations */
void TftPutPixel(screen_control_t *pscr, int x, int y, color_t color);
void TftPutLine(screen_control_t *pscr, int x0, int y0, int x1, int y1, 
                color_t color);

/* Hardware output */
void TftFullScreenWrite(screen_control_t *pscr);
void TftPartialScreenWrite(screen_control_t *pscr, int x, int y, 
                           int width, int height);

/* Helper functions */
uint16_t ColorToRGB565(color_t color);

#ifdef __cplusplus
Image<RGB565>* TftGetTGXImage(screen_control_t *pscr);

}
#endif

// RGB565 palette
static const uint16_t spPalette[8] = 
{
    0x0000, // Black
    0x001F, // Blue
    0xF800, // Red
    0xF81F, // Magenta
    0x07E0, // Green
    0x07FF, // Cyan
    0xFFE0, // Yellow
    0xFFFF  // White
};

#endif