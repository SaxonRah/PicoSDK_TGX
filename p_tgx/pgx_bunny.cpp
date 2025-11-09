// bunny_3d.cpp - 3D Bunny mesh rendering with TGX
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "ili9341/ili9341_tgx.h"
#include "tgx/tgx.h"
#include <stdio.h>
#include <math.h>

// Include the bunny mesh
#include "tgx/example/bunny_fig_small.h"
#define MESH &bunny_fig_small

using namespace tgx;

// Pin definitions for ILI9341 display
#define TFT_MISO  4
#define TFT_CS    5
#define TFT_SCK   6
#define TFT_MOSI  7
#define TFT_RST   8
#define TFT_DC    9

// Framebuffers for double buffering with DMA
static uint16_t framebuffer1[PIX_WIDTH * PIX_HEIGHT];
static uint16_t framebuffer2[PIX_WIDTH * PIX_HEIGHT];
uint16_t* render_fb = framebuffer1;   // Buffer we render to
uint16_t* display_fb = framebuffer2;  // Buffer being sent to display

// Z-buffer for depth testing
static uint16_t zbuffer[PIX_WIDTH * PIX_HEIGHT];

// TGX image wrapper
Image<RGB565> img_render;

// DMA channel for async transfers
int dma_channel = -1;
volatile bool dma_busy = false;

// Only load the shaders we need 
const Shader LOADED_SHADERS = SHADER_PERSPECTIVE | SHADER_ZBUFFER | 
                              SHADER_FLAT | SHADER_GOURAUD | 
                              SHADER_NOTEXTURE | SHADER_TEXTURE_NEAREST | 
                              SHADER_TEXTURE_WRAP_POW2;

// 3D renderer 
Renderer3D<RGB565, LOADED_SHADERS, uint16_t> renderer;

// Hardware config
ili9341_config_t hwConfig;
screen_control_t screen;

// FPS tracking
uint32_t frame_count = 0;
uint64_t last_fps_time = 0;
float current_fps = 0.0f;

// Animation state
int loop_number = 0;

// DMA interrupt handler
void dma_handler() {
    dma_hw->ints0 = 1u << dma_channel;
    dma_busy = false;
}

// Initialize DMA for SPI transfers
void init_dma() {
    dma_channel = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_channel);
    
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi0, true));
    channel_config_set_irq_quiet(&c, false);
    
    dma_channel_configure(
        dma_channel,
        &c,
        &spi_get_hw(spi0)->dr,    // Destination: SPI data register
        NULL,                     // Source: set later
        0,                        // Transfer count: set later
        false                     // Don't start yet
    );
    
    // Enable DMA interrupt
    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

// Non-blocking DMA transfer to display
void display_framebuffer_async(uint16_t* fb) {
    // Wait for any previous DMA to complete
    while(dma_busy) {
        tight_loop_contents();
    }
    
    // Set up display window
    ILI9341_SetOutWriting(screen.mpHWConfig, 0, PIX_WIDTH-1, 0, PIX_HEIGHT-1);
    
    // Byte swap for ILI9341 (big-endian RGB565)
    uint8_t* fb_bytes = (uint8_t*)fb;
    for(int i = 0; i < PIX_WIDTH * PIX_HEIGHT * 2; i += 2) {
        uint8_t temp = fb_bytes[i];
        fb_bytes[i] = fb_bytes[i+1];
        fb_bytes[i+1] = temp;
    }
    
    // Start DMA transfer
    dma_busy = true;
    gpio_put(hwConfig.mGPIO_cs, 0);  // CS low
    dma_channel_set_read_addr(dma_channel, fb, false);
    dma_channel_set_trans_count(dma_channel, PIX_WIDTH * PIX_HEIGHT * 2, true);
}

// Wait for DMA to complete
void wait_for_dma() {
    while(dma_busy) {
        tight_loop_contents();
    }
    gpio_put(hwConfig.mGPIO_cs, 1);  // CS high
}

// Calculate FPS
void update_fps() {
    frame_count++;
    uint64_t now = time_us_64();
    
    if (now - last_fps_time >= 1000000) {
        current_fps = frame_count * 1000000.0f / (now - last_fps_time);
        printf("FPS: %.2f | Mode: ", current_fps);
        
        switch(loop_number % 4) {
            case 0: printf("Gouraud + Texture\n"); break;
            case 1: printf("Wireframe\n"); break;
            case 2: printf("Flat Shading\n"); break;
            case 3: printf("Gouraud Shading\n"); break;
        }
        
        frame_count = 0;
        last_fps_time = now;
    }
}

// Compute model transformation matrix based on time
fMat4 compute_model_matrix(uint32_t time_ms, int& loop_num) {
    const float end1 = 6000;  // Far away duration
    const float end2 = 2000;  // Zoom in duration
    const float end3 = 6000;  // Close up duration
    const float end4 = 2000;  // Zoom out duration
    
    int total_time = (int)(end1 + end2 + end3 + end4);
    
    loop_num = time_ms / total_time;
    float t = time_ms % total_time;
    
    // Continuous rotation: 1 full rotation every 4 seconds
    const float rotation_y = 360.0f * (t / 4000.0f);
    
    // Camera zoom animation
    float tz, ty;
    const float scale = 9.0f;      // Model scale
    const float near_z = 15.0f;    // Close zoom distance
    const float far_z = 25.0f;     // Far distance
    const float up_y = 2.0f;       // Vertical offset for head focus
    
    if (t < end1) {
        // Far away
        tz = -far_z;
        ty = 0;
    } else {
        t -= end1;
        if (t < end2) {
            // Zooming in
            float progress = t / end2;
            tz = -far_z + (far_z - near_z) * progress;
            ty = -up_y * progress;
        } else {
            t -= end2;
            if (t < end3) {
                // Close up
                tz = -near_z;
                ty = -up_y;
            } else {
                // Zooming out
                t -= end3;
                float progress = t / end4;
                tz = -near_z - (far_z - near_z) * progress;
                ty = up_y * (progress - 1);
            }
        }
    }
    
    // Build transformation matrix
    fMat4 M;
    M.setScale({scale, scale, scale});          // Scale model
    M.multRotate(-rotation_y, {0, 1, 0});       // Rotate around Y axis
    M.multTranslate({0, ty, tz});               // Position in space
    
    return M;
}

// Setup function
void setup_3d_renderer() {
    // Setup the image wrapper for current render buffer
    img_render.set((RGB565*)render_fb, PIX_WIDTH, PIX_HEIGHT);
    
    // Configure 3D renderer
    renderer.setViewportSize(PIX_WIDTH, PIX_HEIGHT);
    renderer.setOffset(0, 0);
    renderer.setImage(&img_render);
    renderer.setZbuffer(zbuffer);
    
    // Set perspective projection
    // FOV: 45 degrees, aspect ratio, near: 1.0, far: 100.0
    renderer.setPerspective(45.0f, ((float)PIX_WIDTH) / PIX_HEIGHT, 1.0f, 100.0f);
    
    // Set material properties (bronze with specular highlights)
    renderer.setMaterial(RGBf(0.85f, 0.55f, 0.25f), // Color
                        0.2f,  // Ambient
                        0.7f,  // Diffuse
                        0.8f,  // Specular
                        64);   // Shininess
    
    // Enable backface culling
    renderer.setCulling(1);
    
    // Set texture quality (CORRECTED: use proper constants)
    renderer.setTextureQuality(SHADER_TEXTURE_NEAREST);
    renderer.setTextureWrappingMode(SHADER_TEXTURE_WRAP_POW2);
}

int main() {
    stdio_init_all();
    printf("TGX 3D Bunny Mesh Demo\n");
    
    // Initialize display hardware    
    ILI9341_Init(&hwConfig, spi0, 62 * 1000 * 1000,  // 62.5 MHz SPI
                 TFT_MISO, TFT_CS, TFT_SCK, 
                 TFT_MOSI, TFT_RST, TFT_DC);
    
    TftInitScreen(&screen, &hwConfig, render_fb);
    
    // Initialize DMA
    init_dma();
    
    // Setup 3D renderer
    setup_3d_renderer();
    
    printf("Display initialized: %dx%d\n", PIX_WIDTH, PIX_HEIGHT);
    printf("Starting 3D rendering...\n\n");
    
    last_fps_time = time_us_64();
    
    // Main render loop
    while(1) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        // Compute model transformation
        fMat4 model_matrix = compute_model_matrix(current_time, loop_number);
        renderer.setModelMatrix(model_matrix);
        
        // Clear buffers
        img_render.fillScreen(RGB565_Cyan);  // Clear to cyan background
        renderer.clearZbuffer();              // Clear depth buffer
        
        // Render mesh based on current mode (CORRECTED: use proper constants)
        switch(loop_number % 4) {
            case 0: // Gouraud shading + texture
                renderer.setShaders(SHADER_GOURAUD | SHADER_TEXTURE);
                renderer.drawMesh(MESH, false);
                break;
                
            case 1: // Wireframe
                renderer.drawWireFrameMesh(MESH, true);
                break;
                
            case 2: // Flat shading
                renderer.setShaders(SHADER_FLAT);
                renderer.drawMesh(MESH, false);
                break;
                
            case 3: // Gouraud shading (no texture)
                renderer.setShaders(SHADER_GOURAUD);
                renderer.drawMesh(MESH, false);
                break;
        }
        
        // Start async transfer of render buffer to display
        display_framebuffer_async(render_fb);
        
        // Swap buffers - start rendering next frame while transfer happens
        uint16_t* temp = render_fb;
        render_fb = display_fb;
        display_fb = temp;
        
        // Update image wrapper to new render buffer
        img_render.set((RGB565*)render_fb, PIX_WIDTH, PIX_HEIGHT);
        renderer.setImage(&img_render);
        
        // Update FPS counter
        update_fps();
        
        // If we finish rendering before DMA completes, wait
        wait_for_dma();
    }
    
    return 0;
}