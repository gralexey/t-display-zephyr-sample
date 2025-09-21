#include "conway.h"
#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

/* Field dimensions for default T-Display screen (240x135) */
int field_width = 24;
int field_height = 13;
int cell_size = 10;

static void fill_buf(uint16_t *buf, int x, int y, uint16_t color, int stride) {
    buf[x + y * (stride)] = color;
}

static void draw_cell(int field_x, int field_y, bool alive, uint16_t *buf, int width) {
    for (int i = 0; i < cell_size; i++) {
        for (int j = 0; j < cell_size; j++) {
            fill_buf(buf, (field_x*cell_size + i), (field_y*cell_size + j), alive ? 0x0000 : 0xffff, width);
        }
    }
}

static void reset_buf(uint16_t *buf, int width, int height) {
    memset(buf, 0xffff, width * height * sizeof(uint16_t));
}

static void set_field_to_buf(bool field[field_width][field_height], uint16_t *buf, int height) {
    for (int x = 0; x < field_width; x++) {
        for (int y = 0; y < field_height; y++) {
            draw_cell(x, y, field[x][y], buf, height);
        }
    }
}

static void initialize_field(bool field[field_width][field_height], int seed) {
    int num_cells = 80;
    srand(seed);
    memset(field, false, field_width * field_height * sizeof(bool));
    
    int cells_placed = 0;
    while (cells_placed < num_cells) {
        int x = rand() % field_width;
        int y = rand() % field_height;
        if (field[x][y] == false) {
            field[x][y] = true;
            cells_placed++;
        }
    }
}

static void calc_field_next_step(bool field[field_width][field_height]) {    
    bool next_field[field_width][field_height];
    
    for (int x = 0; x < field_width; x++) {
        for (int y = 0; y < field_height; y++) {
            int live_neighbors = 0;
            
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    
                    int nx = x + dx;
                    int ny = y + dy;
                    
                    if (nx >= 0 && nx < field_width && ny >= 0 && ny < field_height) {
                        if (field[nx][ny]) {
                            live_neighbors++;
                        }
                    }
                }
            }
            
            bool current_state = field[x][y];
            bool next_state = false;
            
            if (current_state) {
                next_state = (live_neighbors == 2 || live_neighbors == 3);
            } else {
                next_state = (live_neighbors == 3);
            }
            
            next_field[x][y] = next_state;
        }
    }
    
    for (int x = 0; x < field_width; x++) {
        for (int y = 0; y < field_height; y++) {
            field[x][y] = next_field[x][y];
        }
    }
}

void conway_run(const struct device *disp, uint16_t *buf, int width, int height, struct display_buffer_descriptor *desc) {
    bool field[field_width][field_height];
    
    unsigned int seed = 78874678;
    initialize_field(field, seed);

    set_field_to_buf(field, buf, height);
    display_write(disp, 0, 0, desc, buf);

    int steps = 1000;
    for (int t = 0; t < steps; t++) {
        calc_field_next_step(field);

        reset_buf(buf, height, width);
        
        set_field_to_buf(field, buf, height);

        display_write(disp, 0, 0, desc, buf);
        
        k_sleep(K_MSEC(50));
    }
}
