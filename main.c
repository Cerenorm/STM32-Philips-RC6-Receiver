#include <misc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_usart.h>

#include "ssd1306.h"
#include "rc6.h"


int main(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	unsigned char ch[10];

	ssd1306_init();
	//Bilgilendirme mesajý ekrana yazýlýyor
	sprintf(ch, "init ssd1306");
	ssd1306_draw_string_16x16(0,0, ch);
	ssd1306_i2c_draw_buffer(I2C1, I2C1_SSD1306_SLAVE_ADDRESS8);


	//Kullanýlan portlar kuruluyor ve deðiþkenlere ilk deðerleri veriliyor
	rc6Init();
	//Bilgilendirme mesajý ekrana yazýlýyor
	sprintf(ch, "rc6 init");
	ssd1306_draw_string_16x16(0,1, ch);
	ssd1306_i2c_draw_buffer(I2C1, I2C1_SSD1306_SLAVE_ADDRESS8);



	//Kullanýlan ledin baðlý olduðu c portunun 13. pini yapýlandýrýlýyor
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	while (1) {
		if (receiver_stage == START_POINT) {

				//Ekran belleði temizleniyor
				ssd1306_clear_display_buffer();

				//Ekranýn 1. satýrýna address içeriði yazýlýyor
				sprintf(ch, "Adr:%03d", address);
				ssd1306_draw_string_16x16(0,0, ch);

				//Ekranýn 2. satýrýna command içeriði yazýlýyor
				sprintf(ch, "Com:%03d", command);
				ssd1306_draw_string_16x16(0,1, ch);

				//Ekranýn 3. satýrýna remote_data içeriði yazýlýyor
				sprintf(ch, "Dat:%03d", remote_data);
				ssd1306_draw_string_16x16(0,2, ch);

				//Ekranýn 4. satýrýna eðer hata var ise hata sayýsý yazýlýyor
				if(err){
					sprintf(ch, "Err:%03d", err);
					ssd1306_draw_string_16x16(0, 3, ch);
				}

				//Bellekteki veriler ekran belleðine aktarýlýyor
				ssd1306_i2c_draw_buffer(I2C1, I2C1_SSD1306_SLAVE_ADDRESS8);

				//Led deðiþkenine göre ledin yanma konumunu deðiþtiriliyor
				if (led) {
					GPIO_ResetBits(GPIOC,GPIO_Pin_13 );
				} else {
					GPIO_SetBits(GPIOC, GPIO_Pin_13);
		}

		}
	}
}
