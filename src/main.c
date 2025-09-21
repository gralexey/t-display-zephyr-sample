
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <stdlib.h>
#include <math.h>
#include "conway.h"

K_HEAP_DEFINE(heap, 80 * 1024);  // 70 KB internal SRAM

// Helper function to check if a point is inside a triangle
static int point_in_triangle(float px, float py, float x1, float y1, float x2, float y2, float x3, float y3) {
    float denom = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
    if (fabs(denom) < 1e-10) return 0;
    
    float a = ((y2 - y3) * (px - x3) + (x3 - x2) * (py - y3)) / denom;
    float b = ((y3 - y1) * (px - x3) + (x1 - x3) * (py - y3)) / denom;
    float c = 1 - a - b;
    
    return (a >= 0 && b >= 0 && c >= 0) ? 1 : 0;
}

// Recursive function to draw Sierpinski triangle
static void draw_sierpinski_triangle(uint16_t *buf, int width, int height, 
                                   float x1, float y1, float x2, float y2, float x3, float y3, 
                                   int depth, uint16_t color) {
    if (depth <= 0) {
        // Draw the triangle by checking each pixel
        float min_x = fmin(fmin(x1, x2), x3);
        float max_x = fmax(fmax(x1, x2), x3);
        float min_y = fmin(fmin(y1, y2), y3);
        float max_y = fmax(fmax(y1, y2), y3);
        
        for (int i = (int)min_x; i <= (int)max_x && i < width; i++) {
            for (int j = (int)min_y; j <= (int)max_y && j < height; j++) {
                if (point_in_triangle(i, j, x1, y1, x2, y2, x3, y3)) {
                    buf[j + i * height] = color;
                }
            }
        }
        return;
    }
    
    // Calculate midpoints
    float mx1 = (x1 + x2) / 2.0f;
    float my1 = (y1 + y2) / 2.0f;
    float mx2 = (x2 + x3) / 2.0f;
    float my2 = (y2 + y3) / 2.0f;
    float mx3 = (x3 + x1) / 2.0f;
    float my3 = (y3 + y1) / 2.0f;
    
    // Recursively draw three smaller triangles
    draw_sierpinski_triangle(buf, width, height, x1, y1, mx1, my1, mx3, my3, depth - 1, color);
    draw_sierpinski_triangle(buf, width, height, mx1, my1, x2, y2, mx2, my2, depth - 1, color);
    draw_sierpinski_triangle(buf, width, height, mx3, my3, mx2, my2, x3, y3, depth - 1, color);
}

static void simple_display_test(const struct device *disp, uint16_t *buf, int width, int height, struct display_buffer_descriptor *desc) {
    // Clear the buffer with black background
    for (int i = 0; i < width * height; i++) {
        buf[i] = 0x0000; // Black background
    }
    
    // Calculate triangle vertices for a centered equilateral triangle
    float center_x = width / 2.0f;
    float center_y = height / 2.0f;
    float size = fmin(width, height) * 0.8f; // 80% of the smaller dimension
    
    // Equilateral triangle vertices
    float x1 = center_x;
    float y1 = center_y - size * 0.5f;
    float x2 = center_x - size * 0.433f; // cos(30°) ≈ 0.866, so 0.866/2 ≈ 0.433
    float y2 = center_y + size * 0.25f;
    float x3 = center_x + size * 0.433f;
    float y3 = center_y + size * 0.25f;
    
    // Draw Sierpinski triangle with 6 levels of recursion
    uint16_t triangle_color = 0xFFFF; // White color
    draw_sierpinski_triangle(buf, width, height, x1, y1, x2, y2, x3, y3, 6, triangle_color);
    
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

