#include <string.h>
#include <stdlib.h>

#include "ili9341/ili9341.h"

// TODO: PSE uncomment one mode only.

//#define MODE_TEST_TEXTBOX_DRAWING
#define MODE_TEST_RANDOM_LINES
//#define MODE_TEST_RANDOM_LABELS

void PRN32(uint32_t *val)
{ 
    *val ^= *val << 13;
    *val ^= *val >> 17;
    *val ^= *val << 5;
}

void TestBoxDraw(screen_control_t *p_screen)
{
    assert_(p_screen);

    static int x = 0, y = 0;
    if(!x && !y)
    {
        memset(p_screen->mpPixBuffer, 0xFF, sizeof(p_screen->mpPixBuffer));
        TftFullScreenWrite(p_screen);
    }
    TftClearRect8(p_screen, x, y);

    if(++x >= TEXT_WIDTH)
    {
        x = 0;
        if(++y >= TEXT_HEIGHT)
            y = 0;
    }
    TftFullScreenSelectiveWrite(p_screen, 16);
}

void TestRandomLabels(screen_control_t *p_screen)
{
    assert_(p_screen);

    static uint32_t rnd_seed = 0xa5efddbd;

    PRN32(&rnd_seed);
    const int x = rnd_seed % 240;
    PRN32(&rnd_seed);
    const int y = rnd_seed % 312;

    TftPutTextLabel(p_screen, "Pico RULEZZ", x, y, false);
    
    TftFullScreenSelectiveWrite(p_screen, 10000);
}

void TestRandomLines(screen_control_t *p_screen)
{
    assert_(p_screen);

    static uint32_t rnd_seed = 0xa5efddbd;

    PRN32(&rnd_seed);
    const int x0 = rnd_seed % 240;
    PRN32(&rnd_seed);
    const int x1 = rnd_seed % 240;
    PRN32(&rnd_seed);
    const int y0 = rnd_seed % 320;
    PRN32(&rnd_seed);
    const int y1 = rnd_seed % 320;

    TftPutLine(p_screen, x0, y0, x1, y1);
    
    TftFullScreenSelectiveWrite(p_screen, 10000);
}

int main() 
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    sleep_ms(250);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    static screen_control_t sScreen =
    {
        .mCursorX = 0,
        .mCursorY = 0,
        .mCursorType = 0,
        .mCanvasPaper = kBlack,
        .mCanvasInk = kWhite
    };

    /*
    DISPLAY
    =======
    Pico pin  Disp.pin   Description
    (pin 36)  VCC        3.3V power input.
    (pin 38)  GND        Ground.
    (pin 07)  CS         LCD chip select signal, low level enable.
    (pin 11)  RESET      LCD reset signal, low level reset.
    (pin 12)  DC/RS      LCD register / data selection signal; high level: register.
    (pin 10)  SDI(MOSI)  SPI bus write data signal.
    (pin 09)  SCK        SPI bus clock signal.
    (pin 36)  LED        Backlight control.
    (pin 06)  SDO(MISO)  SPI bus read data signal. Hasn't been used so far in here.
    
    TOUCH PANEL
    ===========
    Pico pin  Dev.pin    Description
    (pin 20)  T_IRQ      Touch event indicator (active low).
    (pin 15)  T_DIN      SPI MOSI signal.
    (pin 14)  T_CLK      SPI SCK signal.
    (pin 17)  T_CS       Device chip select (active low).
    (pin 16)  T_DO       SPI MISO signal.
    */

    ili9341_config_t ili9341_hw_config;
    sScreen.mpHWConfig = &ili9341_hw_config;
    ILI9341_Init(sScreen.mpHWConfig, spi0, 500 * MHz, 4, 5, 6, 7, 8, 9);

    sScreen.mCanvasPaper = kBlack;
    sScreen.mCanvasInk = kMagenta;
    TftClearScreenBuffer(&sScreen, kBlack, kRed);
    TftFullScreenWrite(&sScreen);

    int led_state = 0;
    for(;;)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, (led_state & 1));
        ++led_state;

#ifdef MODE_TEST_TEXTBOX_DRAWING
        TestBoxDraw(&sScreen);
        sleep_ms(100);
        continue;
#endif
#ifdef MODE_TEST_RANDOM_LINES
        TestRandomLines(&sScreen);
        continue;
#endif
#ifdef MODE_TEST_RANDOM_LABELS
        TestRandomLabels(&sScreen);
        //sleep_ms(250);
        continue;
#endif
    }
}
