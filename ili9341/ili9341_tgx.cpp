///////////////////////////////////////////////////////////////////////////////
//
//  Modified ILI9341 driver implementation for TGX
//
///////////////////////////////////////////////////////////////////////////////
#include "ili9341_tgx.h"
#include <stdlib.h>
#include "font_8x8.h"

// =============================================================================
// C functions - wrapped in extern "C" for C++ compilation
// =============================================================================
extern "C" {

static inline void ILI9341_CS_Set(const ili9341_config_t *pconfig, int state) 
{
    asm volatile("nop \n nop \n nop");
    gpio_put(pconfig->mGPIO_cs, state);
    asm volatile("nop \n nop \n nop");
}

void ILI9341_SetCommand(const ili9341_config_t *pconfig, uint8_t cmd) 
{
    ILI9341_CS_Set(pconfig, CS_ENABLE);
    gpio_put(pconfig->mGPIO_dc, 0);
    asm volatile("nop \n nop \n nop");
    spi_write_blocking(pconfig->mpSPIPort, &cmd, 1);
    gpio_put(pconfig->mGPIO_dc, 1);
    ILI9341_CS_Set(pconfig, CS_DISABLE);
}

void ILI9341_CommandParam(const ili9341_config_t *pconfig, uint8_t data) 
{
    ILI9341_CS_Set(pconfig, CS_ENABLE);
    spi_write_blocking(pconfig->mpSPIPort, &data, 1);
    ILI9341_CS_Set(pconfig, CS_DISABLE);
}

void ILI9341_SetOutWriting(const ili9341_config_t *pconfig,
                            const int start_col, const int end_col,
                            const int start_page, const int end_page)
{
    assert_(pconfig);

    ILI9341_SetCommand(pconfig, ILI9341_CASET);
    ILI9341_CommandParam(pconfig, (start_col >> 8) & 0xFF);
    ILI9341_CommandParam(pconfig, start_col & 0xFF);
    ILI9341_CommandParam(pconfig, (end_col >> 8) & 0xFF);
    ILI9341_CommandParam(pconfig, end_col & 0xFF);

    ILI9341_SetCommand(pconfig, ILI9341_PASET);
    ILI9341_CommandParam(pconfig, (start_page >> 8) & 0xFF);
    ILI9341_CommandParam(pconfig, start_page & 0xFF);
    ILI9341_CommandParam(pconfig, (end_page >> 8) & 0xFF);
    ILI9341_CommandParam(pconfig, end_page & 0xFF);

    ILI9341_SetCommand(pconfig, ILI9341_RAMWR);
}

void ILI9341_WriteData(const ili9341_config_t *pconfig, void *buffer, int bytes)
{
    ILI9341_CS_Set(pconfig, CS_ENABLE);
    spi_write_blocking(pconfig->mpSPIPort, (uint8_t*)buffer, bytes);
    ILI9341_CS_Set(pconfig, CS_DISABLE);
}

void ILI9341_Init(ili9341_config_t *pconfig, spi_inst_t *pspi_port, 
                    int spi_clock_freq, int gpio_MISO, int gpio_CS, 
                    int gpio_SCK, int gpio_MOSI, int gpio_RS, int gpio_DC)
{
    assert_(pconfig);
    assert_(pspi_port);

    pconfig->mpSPIPort = pspi_port;
    pconfig->mGPIO_miso = gpio_MISO;
    pconfig->mGPIO_cs = gpio_CS;
    pconfig->mGPIO_sck = gpio_SCK;
    pconfig->mGPIO_mosi = gpio_MOSI;
    pconfig->mGPIO_reset = gpio_RS;
    pconfig->mGPIO_dc = gpio_DC;

    spi_init(pconfig->mpSPIPort, spi_clock_freq);
    spi_set_baudrate(pconfig->mpSPIPort, spi_clock_freq);

    gpio_set_function(pconfig->mGPIO_miso, GPIO_FUNC_SPI);
    gpio_set_function(pconfig->mGPIO_sck, GPIO_FUNC_SPI);
    gpio_set_function(pconfig->mGPIO_mosi, GPIO_FUNC_SPI);

    gpio_init(pconfig->mGPIO_cs);
    gpio_set_dir(pconfig->mGPIO_cs, GPIO_OUT);
    gpio_put(pconfig->mGPIO_cs, 1);

    gpio_init(pconfig->mGPIO_reset);
    gpio_set_dir(pconfig->mGPIO_reset, GPIO_OUT);
    gpio_put(pconfig->mGPIO_reset, 1);

    gpio_init(pconfig->mGPIO_dc);
    gpio_set_dir(pconfig->mGPIO_dc, GPIO_OUT);
    gpio_put(pconfig->mGPIO_dc, 0);

    sleep_ms(10);
    gpio_put(pconfig->mGPIO_reset, 0);
    sleep_ms(10);
    gpio_put(pconfig->mGPIO_reset, 1);
    sleep_ms(100);

    ILI9341_SetCommand(pconfig, 0x01);
    sleep_ms(100);

    ILI9341_SetCommand(pconfig, ILI9341_GAMMASET);
    ILI9341_CommandParam(pconfig, 0x01);

    ILI9341_SetCommand(pconfig, ILI9341_GMCTRP1);
    uint8_t gamma_p[] = {0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 
                         0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00};
    ILI9341_WriteData(pconfig, gamma_p, 15);

    ILI9341_SetCommand(pconfig, ILI9341_GMCTRN1);
    uint8_t gamma_n[] = {0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 
                         0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f};
    ILI9341_WriteData(pconfig, gamma_n, 15);

    ILI9341_SetCommand(pconfig, ILI9341_MADCTL);
    ILI9341_CommandParam(pconfig, 0x48);
   
    ILI9341_SetCommand(pconfig, ILI9341_PIXFMT);
    ILI9341_CommandParam(pconfig, 0x55);

    ILI9341_SetCommand(pconfig, ILI9341_FRMCTR1);
    ILI9341_CommandParam(pconfig, 0x00);
    ILI9341_CommandParam(pconfig, 0x1B);

    ILI9341_SetCommand(pconfig, ILI9341_SLPOUT);
    ILI9341_SetCommand(pconfig, ILI9341_DISPON);
}

uint16_t ColorToRGB565(color_t color)
{
    return spPalette[color & 0x07];
}

void TftInitScreen(screen_control_t *pscr, ili9341_config_t *pconfig, 
                   uint16_t *framebuffer)
{
    assert_(pscr);
    assert_(pconfig);
    assert_(framebuffer);

    pscr->mpHWConfig = pconfig;
    pscr->mpFrameBuffer = framebuffer;
    pscr->mCursorX = 0;
    pscr->mCursorY = 0;
    pscr->mCanvasPaper = kBlack;
    pscr->mCanvasInk = kWhite;

    // Create TGX Image wrapper (C++ code in C function)
    pscr->mpTGXImage = new Image<RGB565>((RGB565*)framebuffer, PIX_WIDTH, PIX_HEIGHT);
}

void TftClearScreenBuffer(screen_control_t *pscr, color_t paper, color_t ink)
{
    assert_(pscr);
    assert_(pscr->mpFrameBuffer);

    uint16_t color = ColorToRGB565(paper);
    
    for(int i = 0; i < PIX_WIDTH * PIX_HEIGHT; i++) {
        pscr->mpFrameBuffer[i] = color;
    }

    pscr->mCursorX = 0;
    pscr->mCursorY = 0;
    
    if(paper >= 0) {
        pscr->mCanvasPaper = paper;
        pscr->mCanvasInk = ink;
    }
}

void TftSetCursor(screen_control_t *pscr, int x, int y)
{
    assert_(pscr);
    pscr->mCursorX = x;
    pscr->mCursorY = y;
}

void TftPutChar(screen_control_t *pscr, int x, int y, color_t paper, 
                color_t ink, char chr)
{
    assert_(pscr);
    assert_(pscr->mpFrameBuffer);

    if(x < 0 || x >= TEXT_WIDTH || y < 0 || y >= TEXT_HEIGHT)
        return;

    uint16_t paperColor = ColorToRGB565(paper);
    uint16_t inkColor = ColorToRGB565(ink);

    unsigned char c = (unsigned char)chr;
    if(c < 0x20) c = 0x20;
    
    const uint8_t *glyph = &kFONT_[2 + (c - 0x20) * 8];

    int pixX = x * 8;
    int pixY = y * 8;

    for(int row = 0; row < 8; row++) {
        uint8_t line = glyph[row];
        for(int col = 0; col < 8; col++) {
            int px = pixX + col;
            int py = pixY + row;
            
            if(px < PIX_WIDTH && py < PIX_HEIGHT) {
                bool isSet = (line & (0x80 >> col)) != 0;
                pscr->mpFrameBuffer[py * PIX_WIDTH + px] = isSet ? inkColor : paperColor;
            }
        }
    }
}

void TftPutString(screen_control_t *pscr, const char* str, int top_y, 
                  int bot_y, color_t paper, color_t ink)
{
    assert_(pscr);
    assert_(str);

    while(*str) {
        if(*str == '\n' || pscr->mCursorX >= TEXT_WIDTH) {
            pscr->mCursorX = 0;
            pscr->mCursorY++;
        }

        if(pscr->mCursorY >= bot_y) {
            pscr->mCursorY = top_y;
        }

        if(*str != '\n') {
            TftPutChar(pscr, pscr->mCursorX, pscr->mCursorY, paper, ink, *str);
            pscr->mCursorX++;
        }
        str++;
    }
}

void TftPrintf(screen_control_t *pscr, int top_y, int bot_y, color_t paper,
               color_t ink, const char* fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    TftPutString(pscr, buffer, top_y, bot_y, paper, ink);
}

void TftPutPixel(screen_control_t *pscr, int x, int y, color_t color)
{
    assert_(pscr);
    assert_(pscr->mpFrameBuffer);

    if(x >= 0 && x < PIX_WIDTH && y >= 0 && y < PIX_HEIGHT) {
        pscr->mpFrameBuffer[y * PIX_WIDTH + x] = ColorToRGB565(color);
    }
}

void TftPutLine(screen_control_t *pscr, int x0, int y0, int x1, int y1, 
                color_t color)
{
    assert_(pscr);

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while(1) {
        TftPutPixel(pscr, x0, y0, color);

        if(x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if(e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if(e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void TftFullScreenWrite(screen_control_t *pscr)
{
    assert_(pscr);
    assert_(pscr->mpHWConfig);
    assert_(pscr->mpFrameBuffer);

    ILI9341_SetOutWriting(pscr->mpHWConfig, 0, PIX_WIDTH-1, 0, PIX_HEIGHT-1);
    ILI9341_WriteData(pscr->mpHWConfig, pscr->mpFrameBuffer, 
                      PIX_WIDTH * PIX_HEIGHT * 2);
}

void TftPartialScreenWrite(screen_control_t *pscr, int x, int y, 
                           int width, int height)
{
    assert_(pscr);
    assert_(pscr->mpHWConfig);
    assert_(pscr->mpFrameBuffer);

    if(x < 0 || y < 0 || x + width > PIX_WIDTH || y + height > PIX_HEIGHT)
        return;

    ILI9341_SetOutWriting(pscr->mpHWConfig, x, x + width - 1, y, y + height - 1);

    for(int row = 0; row < height; row++) {
        uint16_t *lineStart = &pscr->mpFrameBuffer[(y + row) * PIX_WIDTH + x];
        ILI9341_WriteData(pscr->mpHWConfig, lineStart, width * 2);
    }
}

} // end extern "C"

// =============================================================================
// C++ only functions (not wrapped in extern "C")
// =============================================================================

Image<RGB565>* TftGetTGXImage(screen_control_t *pscr)
{
    assert_(pscr);
    return pscr->mpTGXImage;
}