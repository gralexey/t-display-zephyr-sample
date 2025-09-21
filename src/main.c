
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <stdlib.h>
#include <math.h>
#include "conway.h"

K_HEAP_DEFINE(heap, 80 * 1024);  // 70 KB internal SRAM

static void simple_display_test(const struct device *disp, uint16_t *buf, int width, int height, struct display_buffer_descriptor *desc) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            // Julia set fractal (rotated 90 degrees)
            float x = (i - width/2.0f) / (width/4.0f);
            float y = (j - height/2.0f) / (height/4.0f);
            
            // Julia set parameters
            float cx = -0.7f;  // Real part of constant
            float cy = 0.27015f;  // Imaginary part of constant
            
            float zx = x;
            float zy = y;
            int iter = 0;
            int max_iter = 32;
            
            // Julia set iteration
            while (iter < max_iter && (zx * zx + zy * zy) < 4.0f) {
                float tmp = zx * zx - zy * zy + cx;
                zy = 2.0f * zx * zy + cy;
                zx = tmp;
                iter++;
            }
            
            // Color based on iteration count
            int color_intensity = (iter * 255) / max_iter;
            uint16_t r = (color_intensity >> 3) & 0x1F;  // 5 bits
            uint16_t g = (color_intensity >> 2) & 0x3F;  // 6 bits  
            uint16_t b = (color_intensity >> 3) & 0x1F;  // 5 bits
            buf[j + i * height] = (r << 11) | (g << 5) | b; // RGB565
        }
    }
    display_write(disp, 0, 0, desc, buf);
}
 
int main(void)
{
    const struct device *disp;

    disp = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!disp) {
        printk("Display device not found\n");
        return -1;
    }

    int ret = display_set_pixel_format(disp, PIXEL_FORMAT_RGB_565);
    if (ret < 0) {
        printk("Failed to set pixel format\n");
        return -1;
    }
    display_blanking_off(disp);

    struct display_capabilities capabilities;
    display_get_capabilities(disp, &capabilities);
    int width = capabilities.x_resolution;
    int height = capabilities.y_resolution;

    printf("Display capabilities: %dx%d\n", capabilities.x_resolution, capabilities.y_resolution);

    uint16_t *buf = k_heap_alloc(&heap, 1024, K_NO_WAIT);
    if (buf == NULL) {
        printk("Failed to allocate memory\n");
        return -1;
    }

    int pixels_num = width * height;

    memset(buf, 0xffff, pixels_num * sizeof(uint16_t));

    struct display_buffer_descriptor desc = {
        .buf_size = pixels_num * sizeof(uint16_t),
        .width = height,
        .height = width,
        .pitch = height
    };

#ifdef CONFIG_CONWAY_TEST
    conway_run(disp, buf, width, height, &desc);
#else
    simple_display_test(disp, buf, width, height, &desc);
#endif

    return 0;
}

