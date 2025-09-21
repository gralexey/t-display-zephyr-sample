
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <stdlib.h>
#include <math.h>
#include "conway.h"

K_HEAP_DEFINE(heap, 80 * 1024);  // 70 KB internal SRAM

static void simple_display_test(const struct device *disp, uint16_t *buf, int width, int height, struct display_buffer_descriptor *desc) {
    // Create an artistic geometric pattern with concentric circles and gradients
    int center_x = width / 2;
    int center_y = height / 2;
    int max_radius = (width < height ? width : height) / 2;
    
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            // Calculate distance from center
            int dx = i - center_x;
            int dy = j - center_y;
            int dist_sq = dx * dx + dy * dy;
            int dist = (int)sqrt(dist_sq);
            
            // Create concentric circles with different colors
            int circle_radius = dist / 8;  // 8-pixel wide rings
            int ring_color = circle_radius % 3;
            
            // Create gradient based on angle
            int angle = (int)(atan2(dy, dx) * 180 / 3.14159);
            if (angle < 0) angle += 360;
            int sector = angle / 60;  // 6 sectors of 60 degrees each
            
            // Base colors for each ring type
            int red, green, blue;
            
            switch (ring_color) {
                case 0:  // Red-orange gradient
                    red = 255 - (dist * 255) / max_radius;
                    green = (dist * 128) / max_radius;
                    blue = 0;
                    break;
                case 1:  // Blue-purple gradient
                    red = (dist * 128) / max_radius;
                    green = 0;
                    blue = 255 - (dist * 128) / max_radius;
                    break;
                case 2:  // Green-cyan gradient
                    red = 0;
                    green = 255 - (dist * 128) / max_radius;
                    blue = (dist * 255) / max_radius;
                    break;
                default:
                    red = green = blue = 0;
            }
            
            // Add sector-based color variation
            switch (sector % 3) {
                case 0: red = (red + 128) / 2; break;
                case 1: green = (green + 128) / 2; break;
                case 2: blue = (blue + 128) / 2; break;
            }
            
            // Add some sparkle effect for artistic appeal
            if ((i + j) % 7 == 0) {
                red = (red + 255) / 2;
                green = (green + 255) / 2;
                blue = (blue + 255) / 2;
            }
            
            // Clamp values
            if (red > 255) red = 255;
            if (green > 255) green = 255;
            if (blue > 255) blue = 255;
            
            // Convert to RGB565 format
            uint16_t color = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
            buf[j + i * height] = color;
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

