#ifndef SSD1306_H
#define SSD1306_H



#define I2C_SPEED                   180000                     /*!< I2C Speed */
#define I2C1_SSD1306_SLAVE_ADDRESS8 0x78                      // 8 bit slave address (write)
#define I2C_TIMEOUT                 100000


//
uint8_t global_display_buffer[(128*64)/8];


void ssd1306_init(void);
void ssd1306_clear_display_buffer(void);
void ssd1306_draw_big_string_to_buffer(uint8_t x, uint8_t page_num, uint8_t *string);
int ssd1306_i2c_draw_buffer(I2C_TypeDef *I2Cx, uint8_t slave_address);
void ssd1306_draw_string_to_buffer(uint8_t x, uint8_t page_num, uint8_t *string);

void ssd1306_draw_string_16x16(uint8_t x, uint8_t page_num, uint8_t *string);


#endif
