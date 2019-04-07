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
	//Bilgilendirme mesaj� ekrana yaz�l�yor
	sprintf(ch, "init ssd1306");
	ssd1306_draw_string_16x16(0,0, ch);
	ssd1306_i2c_draw_buffer(I2C1, I2C1_SSD1306_SLAVE_ADDRESS8);


	//Kullan�lan portlar kuruluyor ve de�i�kenlere ilk de�erleri veriliyor
	rc6Init();
	//Bilgilendirme mesaj� ekrana yaz�l�yor
	sprintf(ch, "rc6 init");
	ssd1306_draw_string_16x16(0,1, ch);
	ssd1306_i2c_draw_buffer(I2C1, I2C1_SSD1306_SLAVE_ADDRESS8);



	//Kullan�lan ledin ba�l� oldu�u c portunun 13. pini yap�land�r�l�yor
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	while (1) {
		if (receiver_stage == START_POINT) {

				//Ekran belle�i temizleniyor
				ssd1306_clear_display_buffer();

				//Ekran�n 1. sat�r�na address i�eri�i yaz�l�yor
				sprintf(ch, "Adr:%03d", address);
				ssd1306_draw_string_16x16(0,0, ch);

				//Ekran�n 2. sat�r�na command i�eri�i yaz�l�yor
				sprintf(ch, "Com:%03d", command);
				ssd1306_draw_string_16x16(0,1, ch);

				//Ekran�n 3. sat�r�na remote_data i�eri�i yaz�l�yor
				sprintf(ch, "Dat:%03d", remote_data);
				ssd1306_draw_string_16x16(0,2, ch);

				//Ekran�n 4. sat�r�na e�er hata var ise hata say�s� yaz�l�yor
				if(err){
					sprintf(ch, "Err:%03d", err);
					ssd1306_draw_string_16x16(0, 3, ch);
				}

				//Bellekteki veriler ekran belle�ine aktar�l�yor
				ssd1306_i2c_draw_buffer(I2C1, I2C1_SSD1306_SLAVE_ADDRESS8);

				//Led de�i�kenine g�re ledin yanma konumunu de�i�tiriliyor
				if (led) {
					GPIO_ResetBits(GPIOC,GPIO_Pin_13 );
				} else {
					GPIO_SetBits(GPIOC, GPIO_Pin_13);
		}

		}
	}
}
