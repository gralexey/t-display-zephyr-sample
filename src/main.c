
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <stdlib.h>
#include <math.h>
#include "conway.h"

K_HEAP_DEFINE(heap, 80 * 1024);  

static void simple_display_test(const struct device *disp, uint16_t *buf, int width, int height, struct display_buffer_descriptor *desc) {
    memset(buf, 0xffff, width * height * sizeof(uint16_t));
    
    double a = 1.0;
    double b = 0.1;
    double t_max = 80.0 * 3.14159;
    int center_x = width / 2;
    int center_y = height / 2;
    
    for (double t = 0.0; t < t_max; t += 0.01) {
        double r = a * exp(b * t);
        double x_spiral = r * cos(t);
        double y_spiral = r * sin(t);
        
        int x_screen = center_x + (int)x_spiral;
        int y_screen = center_y - (int)y_spiral;
        
        if (x_screen >= 0 && x_screen < width && y_screen >= 0 && y_screen < height) {
            buf[y_screen + x_screen * height] = 0x0000;
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

