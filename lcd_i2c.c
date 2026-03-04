#include "lcd_i2c.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>

#define I2C_ADDR 0x27

#define LCD_BACKLIGHT 0x08
#define ENABLE 0x04

static int fd;

// ---------- Low-level write ----------
static void lcd_write_byte(char data) {
    write(fd, &data, 1);
}

// Toggle Enable pin
static void lcd_toggle_enable(char data) {
    lcd_write_byte(data | ENABLE);
    usleep(500);
    lcd_write_byte(data & ~ENABLE);
    usleep(500);
}

// Send 4-bit data
static void lcd_send_nibble(char nibble, char mode) {
    char data = nibble | mode | LCD_BACKLIGHT;
    lcd_write_byte(data);
    lcd_toggle_enable(data);
}

// Send full byte
static void lcd_send_byte(char byte, char mode) {
    lcd_send_nibble(byte & 0xF0, mode);
    lcd_send_nibble((byte << 4) & 0xF0, mode);
}

// ---------- Public functions ----------

void lcd_init() {

    fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        printf("I2C open failed\n");
        return;
    }

    ioctl(fd, I2C_SLAVE, I2C_ADDR);

    usleep(50000);

    // Initialization sequence
    lcd_send_nibble(0x30, 0);
    usleep(5000);

    lcd_send_nibble(0x30, 0);
    usleep(1000);

    lcd_send_nibble(0x30, 0);
    usleep(1000);

    lcd_send_nibble(0x20, 0); // 4-bit mode

    lcd_send_byte(0x28, 0); // 2 lines, 5x8 font
    lcd_send_byte(0x0C, 0); // Display ON
    lcd_send_byte(0x06, 0); // Cursor move
    lcd_send_byte(0x01, 0); // Clear display

    usleep(2000);
}

void lcd_clear() {
    lcd_send_byte(0x01, 0);
    usleep(2000);
}

void lcd_set_cursor(int row, int col) {
    int addr = (row == 0) ? 0x80 : 0xC0;
    lcd_send_byte(addr + col, 0);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_send_byte(*str++, 1);
    }
}

void scroll_text_line2(const char *msg)
{
    char window[17];

    int len = strlen(msg);

    if (len <= 16) {
        lcd_set_cursor(1, 0);
        lcd_print(msg);
        return;
    }

    for (int i = 0; i <= len - 16; i++) {

        strncpy(window, msg + i, 16);
        window[16] = '\0';

        lcd_set_cursor(1, 0);
        lcd_print(window);

        usleep(200000);   // 300 ms scroll speed
    }
}