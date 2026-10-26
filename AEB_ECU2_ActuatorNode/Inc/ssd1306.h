#ifndef SSD1306_H_
#define SSD1306_H_
#include <stdint.h>

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_set_pixel(uint8_t x, uint8_t y, uint8_t on);
void ssd1306_update(void);
void ssd1306_draw_char(uint8_t x, uint8_t page, char c);
void ssd1306_draw_string(uint8_t x, uint8_t page, const char *str);
void ssd1306_draw_string_centered(uint8_t page, const char *str);

#endif
