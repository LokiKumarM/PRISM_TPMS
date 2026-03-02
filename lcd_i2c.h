#ifndef LCD_I2C_H
#define LCD_I2C_H

void lcd_init();
void lcd_clear();
void lcd_set_cursor(int row, int col);
void lcd_print(const char *str);
void scroll_text_line2(const char *msg);

#endif