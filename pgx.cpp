#include "pico/stdlib.h"
#include "ili9341/ili9341_tgx.h"
#include "tgx/tgx.h"

using namespace tgx;

// Pin definitions for ILI9341 display
#define TFT_MISO  4
#define TFT_CS    5
#define TFT_SCK   6
#define TFT_MOSI  7
#define TFT_RST   8
#define TFT_DC    9

int main() 
{
    stdio_init_all();

    // Allocate framebuffer (153,600 bytes)
    static uint16_t framebuffer[PIX_WIDTH * PIX_HEIGHT];
    
    // Initialize hardware
    ili9341_config_t hwConfig;
    // ILI9341_Init(&hwConfig, spi0, 500 * MHz, 4, 5, 6, 7, 8, 9);
    ILI9341_Init(&hwConfig, spi0, 50 * 1000 * 1000,  // 50MHz
                 TFT_MISO, TFT_CS, TFT_SCK, 
                 TFT_MOSI, TFT_RST, TFT_DC);
    // Initialize screen control structure
    screen_control_t screen;
    TftInitScreen(&screen, &hwConfig, framebuffer);
    
    // Clear screen
    TftClearScreenBuffer(&screen, kBlack, kWhite);
    
    // *** Use legacy functions ***
    TftPutString(&screen, "Hello from legacy API!", 0, TEXT_HEIGHT, kBlack, kYellow);
    TftPutLine(&screen, 0, 0, 319, 239, kRed);
    TftFullScreenWrite(&screen);
    
    sleep_ms(2000);
    
    // *** Use TGX directly ***
    Image<RGB565>* img = TftGetTGXImage(&screen);
    
    // Clear with TGX
    img->clear(RGB565_Black);
    
    // CORRECTED: Draw with TGX - high quality anti-aliased graphics!
    img->drawThickCircleAA({160.0f, 120.0f}, 80.0f, 3.0f, 
                           RGB565_Red, 1.0f);
    
    // CORRECTED: Thick line with proper EndPath parameters
    img->drawThickLineAA({50.0f, 50.0f}, {270.0f, 190.0f}, 
                         2.0f, 
                         END_ROUNDED, END_ROUNDED,
                         RGB565_Green);
    
    // Or use simple line (no thickness)
    img->drawLineAA({100.0f, 100.0f}, {200.0f, 200.0f}, RGB565_Yellow);
    
    // Fill rectangle
    img->fillRect({10, 10, 100, 50}, RGB565_Blue);

    // Send to display
    TftFullScreenWrite(&screen);
    
    while(1) {
        sleep_ms(1000);
    }
    
    return 0;
}