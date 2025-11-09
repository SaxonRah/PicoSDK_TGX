#include "pico/stdlib.h"
#include "ili9341/ili9341_tgx.h"
#include "tgx/tgx.h"

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
    
    static uint16_t framebuffer[PIX_WIDTH * PIX_HEIGHT];
    
    ili9341_config_t hwConfig;
    // ILI9341_Init(&hwConfig, spi0, 500 * MHz, 4, 5, 6, 7, 8, 9);
    ILI9341_Init(&hwConfig, spi0, 50 * 1000 * 1000,  // 50MHz
                 TFT_MISO, TFT_CS, TFT_SCK, 
                 TFT_MOSI, TFT_RST, TFT_DC);

    screen_control_t screen;
    TftInitScreen(&screen, &hwConfig, framebuffer);
    
    // DIAGNOSTIC: Fill with solid red
    for(int i = 0; i < PIX_WIDTH * PIX_HEIGHT; i++) {
        framebuffer[i] = 0xF800;  // Pure red in RGB565
    }
    
    TftFullScreenWrite(&screen);
    sleep_ms(2000);
    
    // If you see RED, byte order is correct
    // If you see BLUE, byte order is swapped
    // If you see WHITE, there's a deeper issue
    
    while(1) { tight_loop_contents(); }
}