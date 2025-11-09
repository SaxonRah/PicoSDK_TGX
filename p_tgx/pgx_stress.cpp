#include "pico/stdlib.h"
#include "ili9341/ili9341_tgx.h"
#include "tgx/tgx.h"
#include <stdio.h>
#include <math.h>
#include "hardware/clocks.h"

using namespace tgx;


// Pin definitions for ILI9341 display
#define TFT_MISO  4
#define TFT_CS    5
#define TFT_SCK   6
#define TFT_MOSI  7
#define TFT_RST   8
#define TFT_DC    9

// Stress test configuration
#define NUM_CIRCLES 50
#define NUM_LINES 100
#define ANIMATION_SPEED 0.05f

// FPS counter
uint32_t frame_count = 0;
uint64_t last_fps_time = 0;
float current_fps = 0.0f;

void update_fps() {
    frame_count++;
    uint64_t now = time_us_64();
    
    if (now - last_fps_time >= 1000000) {  // Every second
        current_fps = frame_count * 1000000.0f / (now - last_fps_time);
        frame_count = 0;
        last_fps_time = now;
        printf("FPS: %.2f\n", current_fps);
    }
}

// Test 1: Filled shapes stress test
void test_filled_shapes(Image<RGB565>* img, float t) {
    img->clear(RGB565_Black);
    
    // Random filled rectangles
    for(int i = 0; i < 30; i++) {
        int x = (int)(160 + 100 * sin(t + i * 0.3f));
        int y = (int)(120 + 80 * cos(t + i * 0.5f));
        int w = 20 + (int)(10 * sin(t * 2 + i));
        int h = 20 + (int)(10 * cos(t * 2 + i));
        
        RGB565 color((i * 8) & 0x1F, (i * 4) & 0x3F, (i * 8) & 0x1F);
        img->fillRect({x, y, x + w, y + h}, color);
    }
    
    // Overlapping circles
    for(int i = 0; i < 20; i++) {
        fVec2 center(160 + 80 * cos(t + i * 0.4f), 
                     120 + 60 * sin(t + i * 0.6f));
        float radius = 15 + 10 * sin(t * 3 + i);
        
        RGB565 color((i * 13) & 0x1F, (i * 7) & 0x3F, (i * 11) & 0x1F);
        img->fillCircleAA(center, radius, color);
    }
}

// Test 2: Anti-aliased line art
void test_aa_lines(Image<RGB565>* img, float t) {
    img->clear(RGB565_Black);
    
    // Rotating star pattern
    fVec2 center(160, 120);
    for(int i = 0; i < NUM_LINES; i++) {
        float angle = (t + i * 0.02f) * 2.0f;
        float radius = 100 + 20 * sin(t * 2 + i * 0.1f);
        
        fVec2 end(center.x + radius * cos(angle),
                  center.y + radius * sin(angle));
        
        RGB565 color((i * 2) & 0x1F, ((255 - i * 2) >> 2) & 0x3F, ((i * 3) >> 3) & 0x1F);
        img->drawLineAA(center, end, color);
    }
    
    // Thick animated circles
    for(int i = 0; i < 5; i++) {
        float r = 30 + i * 20;
        float thickness = 2.0f + sin(t * 3 + i) * 1.5f;
        RGB565 color(31 - i * 6, 63 - i * 12, 31 - i * 6);
        img->drawThickCircleAA(center, r, thickness, color, 1.0f);
    }
}

// Test 3: Gradient-filled triangles (CORRECTED)
void test_triangles(Image<RGB565>* img, float t) {
    img->clear(RGB565(5, 10, 5));
    
    // Spinning triangles with gradient
    fVec2 center(160, 120);
    for(int i = 0; i < 12; i++) {
        float angle = t + i * (M_PI * 2.0f / 12.0f);
        float r1 = 60, r2 = 100;
        
        fVec2 p1(center.x + r1 * cos(angle),
                 center.y + r1 * sin(angle));
        fVec2 p2(center.x + r2 * cos(angle + 0.3f),
                 center.y + r2 * sin(angle + 0.3f));
        fVec2 p3(center.x + r2 * cos(angle - 0.3f),
                 center.y + r2 * sin(angle - 0.3f));
        
        RGB565 c1((i * 2) & 0x1F, (i * 4) & 0x3F, 31);
        RGB565 c2(31, ((12 - i) * 4) & 0x3F, (i * 2) & 0x1F);
        RGB565 c3((i * 2) & 0x1F, 31, ((12 - i) * 2) & 0x1F);
        
        // Use drawGradientTriangle for gradient effect
        img->drawGradientTriangle(p1, p2, p3, c1, c2, c3);
    }
}

// Test 4: Circle explosion
void test_circles(Image<RGB565>* img, float t) {
    img->clear(RGB565_Black);
    
    for(int i = 0; i < NUM_CIRCLES; i++) {
        float angle = i * (M_PI * 2.0f / NUM_CIRCLES);
        float dist = 50 + 40 * sin(t * 2 + i * 0.2f);
        
        fVec2 pos(160 + dist * cos(angle + t),
                  120 + dist * sin(angle + t));
        
        float radius = 8 + 5 * sin(t * 3 + i * 0.3f);
        
        RGB565 color((int)(31 * (sin(t + i * 0.1f) * 0.5f + 0.5f)),
                     (int)(63 * (cos(t + i * 0.2f) * 0.5f + 0.5f)),
                     (int)(31 * (sin(t * 2 + i * 0.15f) * 0.5f + 0.5f)));
        
        img->fillCircleAA(pos, radius, color);
        img->drawCircleAA(pos, radius + 2, RGB565_White);
    }
}

// Test 5: Thick bezier curves
void test_bezier(Image<RGB565>* img, float t) {
    img->clear(RGB565(10, 10, 30));
    
    // Animated control points
    fVec2 p0(20, 120 + 50 * sin(t));
    fVec2 p1(160, 20 + 40 * cos(t * 1.5f));
    fVec2 p2(160, 220 - 40 * sin(t * 1.3f));
    fVec2 p3(300, 120 + 50 * cos(t * 0.8f));
    
    // Draw thick bezier curves
    for(int i = 0; i < 5; i++) {
        float offset = i * 15.0f;
        fVec2 p1_off = p1 + fVec2(0, offset);
        fVec2 p2_off = p2 + fVec2(0, -offset);
        
        RGB565 color((i * 6) & 0x1F, (31 - i * 5) & 0x3F, (i * 7) & 0x1F);
        
        // Draw bezier with multiple segments for smoothness
        const int segments = 50;
        for(int s = 0; s < segments; s++) {
            float t1 = (float)s / segments;
            float t2 = (float)(s + 1) / segments;
            
            // Cubic bezier formula
            auto bezier = [&](float t) -> fVec2 {
                float u = 1 - t;
                return p0 * (u*u*u) + p1_off * (3*u*u*t) + 
                       p2_off * (3*u*t*t) + p3 * (t*t*t);
            };
            
            img->drawThickLineAA(bezier(t1), bezier(t2), 3.0f,
                               END_ROUNDED, END_ROUNDED, color);
        }
    }
}

// Test 6: Polygon stress test (CORRECTED)
void test_polygons(Image<RGB565>* img, float t) {
    img->clear(RGB565(20, 20, 20));
    
    // Draw rotating polygons
    for(int poly = 0; poly < 8; poly++) {
        int sides = 3 + poly;
        float radius = 30 + poly * 10;
        float cx = 160 + 60 * cos(poly * 0.5f);
        float cy = 120 + 60 * sin(poly * 0.5f);
        float rotation = t + poly * 0.3f;
        
        RGB565 fill_color((poly * 4) & 0x1F, ((7 - poly) * 8) & 0x3F, (poly * 3) & 0x1F);
        RGB565 edge_color = RGB565_White;
        
        // Draw filled polygon using fillTriangle
        for(int i = 0; i < sides; i++) {
            float a1 = rotation + (i * M_PI * 2.0f / sides);
            float a2 = rotation + ((i + 1) * M_PI * 2.0f / sides);
            
            iVec2 p1((int)(cx + radius * cos(a1)), (int)(cy + radius * sin(a1)));
            iVec2 p2((int)(cx + radius * cos(a2)), (int)(cy + radius * sin(a2)));
            iVec2 center((int)cx, (int)cy);
            
            // fillTriangle signature: (P1, P2, P3, interior_color, outline_color, opacity)
            img->fillTriangle(center, p1, p2, fill_color, edge_color);
        }
    }
}

// Test 7: Animated shapes
void test_animated_shapes(Image<RGB565>* img, float t) {
    img->clear(RGB565_Black);
    
    // Pulsing rectangles
    for(int i = 0; i < 10; i++) {
        float size = 20 + 30 * sin(t * 2 + i * 0.5f);
        int x = 32 + i * 28;
        int y = 60;
        
        RGB565 color((i * 3) & 0x1F, ((9 - i) * 6) & 0x3F, (i * 3) & 0x1F);
        img->fillRectAA({(float)x, (float)y, (float)x + size, (float)y + size}, color);
    }
    
    // Spiraling circles
    for(int i = 0; i < 40; i++) {
        float angle = t + i * 0.3f;
        float radius = i * 2.5f;
        fVec2 pos(160 + radius * cos(angle), 120 + radius * sin(angle));
        
        RGB565 color((i * 8) & 0x1F, ((40 - i) * 1) & 0x3F, (i * 8) & 0x1F);
        img->fillCircleAA(pos, 5, color);
    }
    
    // Bouncing balls
    for(int i = 0; i < 8; i++) {
        float x = 40 + i * 35;
        float y = 180 + 40 * sin(t * 3 + i * 0.7f);
        float r = 10 + 5 * cos(t * 2 + i);
        
        RGB565 color(31, ((7 - i) * 8) & 0x3F, (i * 4) & 0x1F);
        img->fillCircleAA({x, y}, r, color);
        img->drawThickCircleAA({x, y}, r + 2, 1.5f, RGB565_White);
    }
}

// Test 8: Rainbow gradient animation
void test_gradient(Image<RGB565>* img, float t) {
    // Full screen gradient fill
    for(int y = 0; y < PIX_HEIGHT; y++) {
        for(int x = 0; x < PIX_WIDTH; x++) {
            float hue = fmodf((x / 320.0f + y / 240.0f + t * 0.1f), 1.0f) * 360.0f;
            
            // HSV to RGB conversion
            float c = 1.0f;
            float h2 = hue / 60.0f;
            float x_val = c * (1 - fabsf(fmodf(h2, 2.0f) - 1));
            
            float r, g, b;
            if(h2 < 1) { r = c; g = x_val; b = 0; }
            else if(h2 < 2) { r = x_val; g = c; b = 0; }
            else if(h2 < 3) { r = 0; g = c; b = x_val; }
            else if(h2 < 4) { r = 0; g = x_val; b = c; }
            else if(h2 < 5) { r = x_val; g = 0; b = c; }
            else { r = c; g = 0; b = x_val; }
            
            RGB565 color((uint8_t)(r * 31), (uint8_t)(g * 63), (uint8_t)(b * 31));
            img->drawPixel({x, y}, color);
        }
    }
}

int main() 
{
    //set_sys_clock_khz(300000, true);  // 250 MHz instead of 125 MHz
    
    stdio_init_all();
    
    printf("TGX Stress Test Starting...\n");
    
    // Allocate framebuffer
    static uint16_t framebuffer[PIX_WIDTH * PIX_HEIGHT];
    
    // Initialize hardware with YOUR correct pins
    ili9341_config_t hwConfig;
    // ILI9341_Init(&hwConfig, spi0, 500 * MHz, 4, 5, 6, 7, 8, 9);
    ILI9341_Init(&hwConfig, spi0, 50 * 1000 * 1000,  // 50MHz
                 TFT_MISO, TFT_CS, TFT_SCK, 
                 TFT_MOSI, TFT_RST, TFT_DC);
    
    // Initialize screen
    screen_control_t screen;
    TftInitScreen(&screen, &hwConfig, framebuffer);
    
    // Get TGX image
    Image<RGB565>* img = TftGetTGXImage(&screen);
    
    printf("Display initialized. Starting stress tests...\n");
    
    last_fps_time = time_us_64();
    float t = 0.0f;
    int test_number = 0;
    uint32_t test_start_time = time_us_32() / 1000;
    
    while(1) {
        // Cycle through tests every 5 seconds
        uint32_t current_time = time_us_32() / 1000;
        if(current_time - test_start_time > 5000) {
            test_number = (test_number + 1) % 8;
            test_start_time = current_time;
            printf("Switching to test %d\n", test_number + 1);
        }
        
        // Run current test
        switch(test_number) {
            case 0: test_filled_shapes(img, t); break;
            case 1: test_aa_lines(img, t); break;
            case 2: test_triangles(img, t); break;
            case 3: test_circles(img, t); break;
            case 4: test_bezier(img, t); break;
            case 5: test_polygons(img, t); break;
            case 6: test_animated_shapes(img, t); break;
            case 7: test_gradient(img, t); break;
        }
        
        // Send to display
        TftFullScreenWrite(&screen);
        
        // Update animation time and FPS
        t += ANIMATION_SPEED;
        update_fps();
    }
    
    return 0;
}