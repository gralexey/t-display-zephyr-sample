# T-Display (ST7789) with Zephyr 4.2.99

This is a sample project demonstrating how to set up and use a T-Display esp32 board (ST7789) with Zephyr RTOS 4.2.99. 

## Hardware Requirements

- T-Display with ST7789 controller (240x135 resolution)
- USB cable for programming and power

## Building and Flashing

To build and flash the project to your ESP32 DevKitC with T-Display:

```bash
west build -p always -b esp32_devkitc/esp32/procpu -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc_st7789v.overlay
west flash --esp-baud-rate 460800
```

## Prerequisites

- Zephyr SDK installed
- West tool installed
- ESP32 toolchain configured (`west blobs fetch hal_espressif`)
- T-Display board (ST7789) connected to your computer

## Troubleshooting

If you encounter issues:
1. Ensure the T-Display is properly connected to the ESP32
2. Check that the device tree overlay is correctly applied (`esp32_devkitc_st7789v.overlay`)
3. Try to lower the baud rate (`--esp-baud-rate`) if there is an issue with flashing
4. Make sure Zephyr 4.2.99 is properly installed and initialized (`~/zephyrproject/zephyr/zephyr-env.sh`)
5. Ensure the display backlight is enabled (`backlight` with GPIO 4)

---
