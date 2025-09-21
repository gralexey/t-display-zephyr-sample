
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <stdlib.h>
#include <math.h>
#include "conway.h"

K_HEAP_DEFINE(heap, 80 * 1024);  

static void simple_display_test(const struct device *disp, uint16_t *buf, int width, int height, struct display_buffer_descriptor *desc) {
    memset(buf, 0xffff, width * height * sizeof(uint16_t));
    
    // Calculate diamond size - smaller diamonds for multiple pattern
    int diamond_size = (height < width) ? height / 6 : width / 6;
    if (diamond_size < 3) diamond_size = 3; // Minimum size
    
    // Calculate grid dimensions for multiple diamonds
    int diamonds_x = width / (diamond_size * 2);
    int diamonds_y = height / (diamond_size * 2);
    
    // Draw multiple diamonds in a grid pattern
    for (int grid_y = 0; grid_y < diamonds_y; grid_y++) {
        for (int grid_x = 0; grid_x < diamonds_x; grid_x++) {
            int center_x = (grid_x * diamond_size * 2) + diamond_size;
            int center_y = (grid_y * diamond_size * 2) + diamond_size;
            
            // Draw upper triangle
            for (int i = 1; i <= diamond_size; i++) {
                for (int j = 1; j <= (2*i-1); j++) {
                    int x_screen = (center_x - i + j);
                    int y_screen = center_y - diamond_size + i - 1;
                    if(x_screen >= 0 && x_screen < width && y_screen >= 0 && y_screen < height)
                        buf[y_screen + x_screen * height] = 1;  
                }
            }

            // Draw lower triangle
            for (int i = diamond_size-1; i >= 1; i--) {
                for (int j = 1; j <= (2*i-1); j++) {
                    int x_screen = (center_x - i + j);
                    int y_screen = center_y + (diamond_size - i - 1);
                    if(x_screen >= 0 && x_screen < width && y_screen >= 0 && y_screen < height)
                        buf[y_screen + x_screen * height] = 1; 
                }
            }
        }
    }
    
    display_write(disp, 0, 0, desc, buf);
}
 
int main(void) {
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

