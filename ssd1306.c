#include "stm32f10x.h"
#include <stm32f10x_i2c.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>


#include "myfont.h"
#include "ssd1306.h"


int i2c_send_command(I2C_TypeDef *I2Cx, uint8_t slave_address, uint8_t slave_data){
// Sends I2C data over I2Cx:
//  1) Sends Start Condition. Checks for I2C EV5
//  2) Sends 7 bit address & checks for EV6
//  3) Sends 8 bit command byte (0x00) & checks for EV8
//  4) Sends 8 bits (1 byte) of data & checks for EV8
//  5) Sends Stop Condition
    int TimeOut;

    #define COMMAND_BYTE 0x00

    /* Send I2C1 START condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on I2C1 EV5 and clear it */
    TimeOut = I2C_TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        TimeOut--;
        if (TimeOut == 0){
            return 1;
        }
    }

    /* Send SSD1306 7 bit slave Address for write. Check to make sure ACK received */
    I2C_Send7bitAddress(I2Cx, I2C1_SSD1306_SLAVE_ADDRESS8, I2C_Direction_Transmitter);

    //Test on I2C1 EV6 and clear it
    TimeOut = I2C_TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        TimeOut--;
        if (TimeOut == 0){
            // Send I2C1 STOP Condition
            I2C_GenerateSTOP(I2Cx, ENABLE);
            return 2;
        }
    }
    I2C_SendData(I2Cx, COMMAND_BYTE);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){ // Wait for EV8
        TimeOut--;
        if (TimeOut == 0){
            // Send I2C1 STOP Condition
            I2C_GenerateSTOP(I2Cx, ENABLE);
            return 2;
        }
    }
    I2C_SendData(I2Cx, slave_data);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){ // Wait for EV8
        TimeOut--;
        if (TimeOut == 0){
            // Send I2C1 STOP Condition
            I2C_GenerateSTOP(I2Cx, ENABLE);
            return 2;
        }
    }
    I2C_GenerateSTOP(I2Cx, ENABLE);
    return 0;
}

void ssd1306_i2c_init(I2C_TypeDef *I2Cx, uint8_t ssd1306_slave_address){

// Sends the init commands to the display
    i2c_send_command(I2Cx, ssd1306_slave_address, 0xAE);

    i2c_send_command(I2Cx, ssd1306_slave_address, 0x00 | 0x0);      // low col = 0

    i2c_send_command(I2Cx, ssd1306_slave_address, 0x10 | 0x0);      // hi col = 0
    i2c_send_command(I2Cx, ssd1306_slave_address, 0x40 | 0x0);      // line #0

    i2c_send_command(I2Cx, ssd1306_slave_address, 0x81);            // Set Contrast 0x81
    i2c_send_command(I2Cx, ssd1306_slave_address, 0xCF);
    i2c_send_command(I2Cx, ssd1306_slave_address, 0xA1);            // Segremap - 0xA1
    i2c_send_command(I2Cx, ssd1306_slave_address, 0xC8);            // COMSCAN DEC 0xC8 C0
    i2c_send_command(I2Cx, ssd1306_slave_address, 0xA6);            // Normal Display 0xA6 (Invert A7)

    i2c_send_command(I2Cx, ssd1306_slave_address, 0xA4);            // DISPLAY ALL ON RESUME - 0xA4
    i2c_send_command(I2Cx, ssd1306_slave_address, 0xA8);            // Set Multiplex 0xA8
    i2c_send_command(I2Cx, ssd1306_slave_address, 0x3F);            // 1/64 Duty Cycle

    i2c_send_command(I2Cx, ssd1306_slave_address, 0xD3);            // Set Display Offset 0xD3
    i2c_send_command(I2Cx, ssd1306_slave_address, 0x0);             // no offset

    i2c_send_command(I2Cx, ssd1306_slave_address, 0xD5);            // Set Display Clk Div 0xD5
    i2c_send_command(I2Cx, ssd1306_slave_address, 0x80);            // Recommneded resistor ratio 0x80

    i2c_send_command(I2Cx, ssd1306_slave_address, 0xD9);            // Set Precharge 0xd9
    i2c_send_command(I2Cx, ssd1306_slave_address, 0xF1);

    i2c_send_command(I2Cx, ssd1306_slave_address, 0xDA);            // Set COM Pins0xDA
    i2c_send_command(I2Cx, ssd1306_slave_address, 0x12);

    i2c_send_command(I2Cx, ssd1306_slave_address, 0xDB);            // Set VCOM Detect - 0xDB
    i2c_send_command(I2Cx, ssd1306_slave_address, 0x40);

    i2c_send_command(I2Cx, ssd1306_slave_address, 0x20);            // Set Memory Addressing Mode
    i2c_send_command(I2Cx, ssd1306_slave_address, 0x00);            // 0x00 - Horizontal

    i2c_send_command(I2Cx, ssd1306_slave_address, 0x40 | 0x0);      // Set start line at line 0 - 0x40

    i2c_send_command(I2Cx, ssd1306_slave_address, 0x8D);            // Charge Pump -0x8D
    i2c_send_command(I2Cx, ssd1306_slave_address, 0x14);


    i2c_send_command(I2Cx, ssd1306_slave_address, 0xA4);            //--turn on all pixels - A5. Regular mode A4
    i2c_send_command(I2Cx, ssd1306_slave_address, 0xAF);            //--turn on oled panel - AF
}

int ssd1306_i2c_draw_buffer(I2C_TypeDef *I2Cx, uint8_t slave_address){
    #define SSD1306_COLUMNADDR  0x21
    #define SSD1306_PAGEADDR    0x22
    #define DATA_BYTE           0x40

    int TimeOut;

    uint8_t *buffer_pointer=global_display_buffer;


    i2c_send_command(I2Cx, slave_address, SSD1306_COLUMNADDR);
    i2c_send_command(I2Cx, slave_address, 0x00);            // Column Start address
    i2c_send_command(I2Cx, slave_address, 127);             // Column end address

    i2c_send_command(I2Cx, slave_address, SSD1306_PAGEADDR);
    i2c_send_command(I2Cx, slave_address, 0x00);            // Page Start address
    i2c_send_command(I2Cx, slave_address, 0x07);            // Page end address

    uint8_t x, y;


    /* Send I2C1 START condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on I2C1 EV5 and clear it */
    TimeOut = I2C_TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        TimeOut--;
        if (TimeOut == 0){
            return 1;
        }
    }

    /* Send SSD1306 7 bit slave Address for write. Check to make sure ACK received */
    I2C_Send7bitAddress(I2Cx, slave_address, I2C_Direction_Transmitter);

    //Test on I2C1 EV6 and clear it
    TimeOut = I2C_TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        TimeOut--;
        if (TimeOut == 0){
            // Send I2C1 STOP Condition
            I2C_GenerateSTOP(I2Cx, ENABLE);

            return 2;
        }
    }

    I2C_SendData(I2Cx, DATA_BYTE);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){ // Wait for EV8
        TimeOut--;
        if (TimeOut == 0){
            // Send I2C1 STOP Condition
            I2C_GenerateSTOP(I2Cx, ENABLE);
            return 2;
        }
    }

    for(y=0; y<8; y++){
        for(x=0; x<128; x++){
            //I2C_SendData(I2Cx,0xff);
            I2C_SendData(I2Cx, buffer_pointer[(128*y)+x]);

            while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){ // Wait for EV8
                TimeOut--;
                if (TimeOut == 0){
                    // Send I2C1 STOP Condition
                    I2C_GenerateSTOP(I2Cx, ENABLE);

                    return 2;
                }
            }

        }
    }
    I2C_GenerateSTOP(I2Cx, ENABLE);
    return 0;
}

void ssd1306_clear_display_buffer(void){
// Clear all bytes of the 128x64 display buffer given by buffer_pointer
	uint8_t *buffer_pointer=global_display_buffer;
	uint8_t x, page_index = 0;
    #define NUM_PAGES       8
    #define SSD1306_XRES    128

    for(page_index=0; page_index < NUM_PAGES; page_index++){
        for(x=0; x<SSD1306_XRES; x++){
            buffer_pointer[(128*page_index)+x] = 0x00;
        }
    }
}

void ssd1306_draw_pixel_to_buffer(uint8_t x, uint8_t y, uint8_t* buffer_pointer){
// Turns on the pixel at (x,y) in the local buffer
    #define SSD1306_HORIZONTAL_RES  128

    uint8_t which_bit, which_page, bit_mask;
    uint16_t which_byte;

    which_page = y / 8;             // There are 8 vertical pixels per page
    which_byte = x + (SSD1306_HORIZONTAL_RES * which_page);
    which_bit = y % 8;

    bit_mask = (1<<which_bit);

    buffer_pointer[which_byte] = (buffer_pointer[which_byte] & ~bit_mask) | ((1<<which_bit) & bit_mask);
}





void ssd1306_draw_char_to_buffer(uint8_t x, uint8_t page_num, uint8_t which_char, uint8_t *buffer_pointer){
// Draws 'which_char' to the display buffer given by buffer_pointer at coord (x, page_num)

    uint8_t i;
    uint16_t which_byte = 0;

    //GPIO_WriteBit(GPIOC, GPIO_Pin_9, 0);       // Error LED

    //which_byte = x + ((page_num/8)*128);
    which_byte = x + ((page_num)*128);

    for(i=0; i<5; i++){
        buffer_pointer[which_byte+i] = Ascii_8x5_font[which_char-ASCII_8x5_OFFSET][i];
        //turn_on_error_led_pin();       // Error LED
    }

}
void ssd1306_draw_big_char_to_buffer(uint8_t x, uint8_t page_num, uint8_t which_char, uint8_t *buffer_pointer){
// Draws 'which_char' to the display buffer given by buffer_pointer at coord (x, page_num)
    uint8_t i, j;
    uint16_t which_byte = 0;

    which_byte = x + ((page_num)*128);

    for(j=0; j<5; j++){
    // 40 bits (5 bytes) tall
        which_byte = x + ((page_num+j)*128);
        for(i=0; i<24; i++){
        // 24 bits wide
            buffer_pointer[which_byte+i] = Tahoma24x40[which_char-ASCII_24x40_FONT_ASCII_OFFSET][(24*j)+i];
            //turn_on_error_led_pin();       // Error LED
        }
    }

}


int slide_buf(int c){
	int i=0;
	int j=0;
	for(i=0;i<(128*(8-c));i++){
	   global_display_buffer[i]=global_display_buffer[i+128*c];
	}
	for(i=128*(8-c);i<128*8;i++){
	   global_display_buffer[i]=0;
	}
	return 0;
}

void ssd1306_draw_string_16x16(uint8_t x, uint8_t page_num, uint8_t *string){
	int p=0;
	int j=0;
	int i=0;

	int w=0;

	if(page_num>3){
		slide_buf(2);
		page_num=3;
	}
	for(i=128*page_num*2;i<(128*(page_num+1)*2);i++){
	   global_display_buffer[i]=0;
	}
	p=(page_num*256);
	while(*string){
		w=Ascii_16x16_font[(*string)-' '][24]+2;
		if(((*string)>='0') && ((*string)<='9')){
			w=12;
		}
        if(w>12){
        	w=12;
        }

        if((x+w-2)>128){
        	page_num++;
        	if(page_num>3){
        		slide_buf(2);
        		page_num=3;
        	}
        	for(i=128*page_num*2;i<(128*(page_num+1)*2);i++){
        	   global_display_buffer[i]=0;
        	}
        	x=0;
        }else if((w+x)>128){
        	w-=2;
        }
        p=page_num*256;
		for(j=0;j<w;j++){
			global_display_buffer[x+j+p]=Ascii_16x16_font[(*string)-' '][j*2+1];
			global_display_buffer[x+j+128+p]=Ascii_16x16_font[(*string)-' '][j*2];
		}
		string++;
		x+=w;
	}
}





void ssd1306_draw_string_to_buffer(uint8_t x, uint8_t page_num, uint8_t *string){
	int p=0;
	int j=0;
	int i=0;

	int w=0;

	if(page_num>7){
		slide_buf(1);
		page_num=7;
	}
	for(i=128*page_num;i<(128*(page_num+1));i++){
	   global_display_buffer[i]=0;
	}
	p=(page_num*128);
	while(*string){
		w=5;

        if((x+w)>128){
        	page_num++;
        	if(page_num>7){
        		slide_buf(1);
        		page_num=7;
        	}
        	for(i=128*page_num;i<(128*(page_num+1));i++){
        	   global_display_buffer[i]=0;
        	}
        	x=0;
        }

		for(j=0;j<5;j++){
			global_display_buffer[x+j+p]=Ascii_8x5_font[*string-ASCII_8x5_OFFSET][j];
		}
		string++;
		x+=(w+1);
	}
}


void ssd1306_draw_big_string_to_buffer(uint8_t x, uint8_t page_num, uint8_t *string){
// Draws 'string' to the display buffer (buffer pointer) at coord (x, page_num)
	uint8_t *buffer_pointer=global_display_buffer;
    uint8_t i=0;
    while(string[i] != 0){
        ssd1306_draw_big_char_to_buffer(x+(ASCII_24x40_FONT_WIDTH*i), page_num, string[i], buffer_pointer);
        i++;
    }
}


void init_i2c1_peripheral(I2C_TypeDef *I2Cx){
// Initializes the I2C1 Peripheral on PB6 & PB7
// 1) Enables the GPIOB Peripheral Clock
// 2) Enable the I2C1 Peripheral Clock
// 3) Configure the GPIO peripheral with GPIO_InitStructure and GPIO_Init()
// 4) Configure the I2C1 peripheral with I2C_InitStructure and I2C_Init()

    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    /* GPIOB Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* I2C1 and I2C2 Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /* Configure I2C1 pins: SCL and SDA ----------------------------------------*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // Open Drain, I2C bus pulled high externally
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Enable I2C1 -------------------------------------------------------------*/
    I2C_DeInit(I2C1);

    I2C_Cmd(I2C1, ENABLE);

    /* I2C1 configuration ------------------------------------------------------*/
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x039;                                  // This is important for some reason!
    //I2C_InitStructure.I2C_OwnAddress1 = 0x000;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
    I2C_Init(I2C1, &I2C_InitStructure);
}


void ssd1306_init(void){

    /*----- Configure I2C Phase -----*/
    init_i2c1_peripheral(I2C1);                                               // To Do - Move All this into init_i2c1_peripheral()

    /*----- Transmission Phase -----*/
    // Init sequence for 128x64 OLED module
    ssd1306_i2c_init(I2C1, I2C1_SSD1306_SLAVE_ADDRESS8);

    ssd1306_clear_display_buffer();
}
