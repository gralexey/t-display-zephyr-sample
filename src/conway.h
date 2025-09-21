#ifndef CONWAY_H
#define CONWAY_H

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

void conway_run(const struct device *disp, uint16_t *buf, int width, int height, struct display_buffer_descriptor *desc);

#endif // CONWAY_H
