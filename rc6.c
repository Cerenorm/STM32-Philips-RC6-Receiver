#include <misc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include "rc6.h"


//Fonksiyon içinde kullanýlan deðiþkenler
volatile int bit_counter;
volatile int tt;
volatile int bit_switch;


//Kesme fonksiyonu
void TIM4_IRQHandler(void) {
	int t = 0;
	int j = 0;
	int temp_bit = 0;

	if (TIM_GetFlagStatus(TIM4, TIM_FLAG_Update) != RESET)
		TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);

	//Kararlý bir okuma için bit deðiþim anýndan kaçýnarak
	//bitin baslangýcýndan deðil 1/3 den analiz edeceðiz,
	//bunun için yaklasýk 150 mikrosaniyelik
	//zaman gecikmesi oluþturuyoruz.
	for (j = 0; j < 833; j++) {
		;
	}

	//B portunun 0. pini okunuyor.
	//IR alýcýnýn baglý oldugu pin
	t = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);

	//RC6 IR verici sinyali farklý bileþenlerden oluþuyor. Bunlar sýra ile soyle:
	//Baslýk,adres,komut,veri
	//Bit genisligi 444 mikrosaniye
	//baslangýç 6 bit geniþliðinde bir palsten oluþuyor aralarýnda boþluk bulunmuyor
	//Sinyalin baþýný belirlemek için kullanýlýyor.
	//Daha sonra 2 boþluk ile bunu takip eden  manchester kodlama ile
	//kodlanmýþ bir bit ile baþlýk kýsmý tamamlanýyor.

	switch (receiver_stage) {
	case START_POINT:
		if (t) {
			bit_counter++;
		} else {
			bit_counter = 0;
			return;
		}
		//Baslýk kýsmý aranýyor aralýksýz 6 tane 1
		if (bit_counter > 5) {
			receiver_stage = READ_FIRST_BIT;
			bit_counter = 0;
			led ^= 1;
		}
		break;
	case READ_FIRST_BIT:
		if (t == 0) {
			//Senkronizasyonu saðlamak için ilk bosluk kullanýlýyor
			//j sayacý zaman aþýmý oluþturmak için kullanýlýyor.
			j=0;
			while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0 && j < 10000) {
				//Yeniden kesme çaðrýsý oluþumunu engellemek ve senkronizasyon saðlamak
				//için timer sýfýrlanýyor.
				TIM4->CNT = 0;
				j++;
			}
			//Ýþlem zaman aþýmýna uðramadan biter ise bir sonraki aþamaya geçiliyor.
			if (j < 10000) {
				receiver_stage = SNC_RECEIVE_DATA;
				bit_counter=0;
			} else {
				//Ýþlem zaman aþýmýna uðradý program resetleniyor.
				receiver_stage = START_POINT;
				err++;
			}
			return;
		}
		break;
	case SNC_RECEIVE_DATA:
		//Baþlik ile adres verisi arasinda 12 bitlik bosluk var bunu atliyoruz
		if (bit_counter > 11) {
			receiver_stage = READY_TO_READ;
			command = 0;
			address = 0;
			remote_data = 0;
			bit_switch = 0;
			bit_counter=0;
			tt=0;
			return;
		}
		bit_counter++;

		break;

	case READY_TO_READ:
			//Manchester decoder
			//8bit adres 8bit komut okunuyor manchester kodlama olduðu icin
			//1 bit iki aþamadan oluþuyor toplamda 16*2 32 bit.
		    //0 dan 1 e geçiþ 0, 1 den 0 a geçiþ 1 olarak deðerlendiriliyor.
		    //1 den 1 e ya da 0 dan 0 a geçiþ var ise hatalý giriþ olduðu için
		    //hata sayacý bir arttýrýlýyor ve döngüden çýkýlýyor.

			if (bit_counter < 32) {

				if (bit_switch) {
					if (t == 0) {
						if (tt == 1) {
							temp_bit = 1;
						} else {
							//error
							err++;
							receiver_stage = START_POINT;
							return;
						}
					} else if (t == 1) {
						if (tt == 0) {
							temp_bit = 0;
						} else {
							//error
							err++;
							receiver_stage = START_POINT;
							return;
						}
					}
					if (bit_counter < 16) {
						address = (address << 1)| temp_bit;
					} else{
						command = (command << 1)| temp_bit;
					}
					bit_switch = 0;
					}
			else {
					tt = t;
					bit_switch = 1;
				}
				bit_counter++;
				return;
			}

        //Standart RC6 protokole göre bu verinin command kýsmýnda yer almasý gerekir
		//Asýl ihtiyacýmýz olan veri bu alanda bulunuyor 52-60 aralýðý 8bit
			if (bit_counter < 60) {
				if (bit_counter >51) {
					remote_data = remote_data << 1;
					remote_data = remote_data | t;
				}
				bit_counter++;
			} else {
				receiver_stage = START_POINT;
				err = 0;
			}
		break;
	}
}


void rc6Init(void){
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	//IR alýcýnýn baðlý olduðu B portunun 0. pinini giriþ olarak yapýlandýrýyoruz.
	//Eðer IR alýcý baþka bir porta baðlý ise buradaki port numarasýný
	//ve kesme fonksiyonu içinde okuma yapýlan portu deðiþtirin.

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//Timer 444 mikrosaniyede bir kesme üretecek þekilde yapýlandýrýlýyoruz.
	//Ayrýca senkronizasyon saðlamak amacý ile kesme fonksiyonu içerisinde timer sýfýrlanýyor.
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_InitStructure.TIM_Prescaler = 72;
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStructure.TIM_Period = 444;
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4, &TIM_InitStructure);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);


	//Kesme fonksiyonu ayarlanýyor
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM4, ENABLE);

	//Deðiþkenlere baþlangýç deðerleri veriliyor.
	led = 1;
	receiver_stage = START_POINT;
	tt = 0;
	bit_switch = 0;
	err = 0;
	command = 0;
	address = 0;
	remote_data = 0;
	bit_counter = 0;

}

