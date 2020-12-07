//FROM https://github.com/geoffreymbrown/STM32-Template/blob/master/Library/ST7735.h

#ifndef ST7335_H
#define ST7335_H

/*  Colors are 565 RGB (5 bits Red, 6 bits green, 5 bits blue) */

#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F       
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

#define SPILCD SPI2

/* MADCTL [MY MX MV]
 *    MY  row address order   1 (bottom to top), 0 (top to bottom)
 *    MX  col address order   1 (right to left), 0 (left to right)
 *    MV  col/row exchange    1 (exchange),      0 normal
 */

#define MADCTLGRAPHICS 0x6
#define MADCTLBMP      0x2

#define ST7735_width  128
#define ST7735_height 160

void ST7735_setAddrWindow(uint16_t x0, uint16_t y0, 
			  uint16_t x1, uint16_t y1, uint8_t madctl);
void ST7735_pushColor(uint16_t *color, int cnt);
void ST7735_init();

#endif
