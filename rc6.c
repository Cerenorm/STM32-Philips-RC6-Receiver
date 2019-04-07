#include <misc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include "rc6.h"


//Fonksiyon i�inde kullan�lan de�i�kenler
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

	//Kararl� bir okuma i�in bit de�i�im an�ndan ka��narak
	//bitin baslang�c�ndan de�il 1/3 den analiz edece�iz,
	//bunun i�in yaklas�k 150 mikrosaniyelik
	//zaman gecikmesi olu�turuyoruz.
	for (j = 0; j < 833; j++) {
		;
	}

	//B portunun 0. pini okunuyor.
	//IR al�c�n�n bagl� oldugu pin
	t = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);

	//RC6 IR verici sinyali farkl� bile�enlerden olu�uyor. Bunlar s�ra ile soyle:
	//Basl�k,adres,komut,veri
	//Bit genisligi 444 mikrosaniye
	//baslang�� 6 bit geni�li�inde bir palsten olu�uyor aralar�nda bo�luk bulunmuyor
	//Sinyalin ba��n� belirlemek i�in kullan�l�yor.
	//Daha sonra 2 bo�luk ile bunu takip eden  manchester kodlama ile
	//kodlanm�� bir bit ile ba�l�k k�sm� tamamlan�yor.

	switch (receiver_stage) {
	case START_POINT:
		if (t) {
			bit_counter++;
		} else {
			bit_counter = 0;
			return;
		}
		//Basl�k k�sm� aran�yor aral�ks�z 6 tane 1
		if (bit_counter > 5) {
			receiver_stage = READ_FIRST_BIT;
			bit_counter = 0;
			led ^= 1;
		}
		break;
	case READ_FIRST_BIT:
		if (t == 0) {
			//Senkronizasyonu sa�lamak i�in ilk bosluk kullan�l�yor
			//j sayac� zaman a��m� olu�turmak i�in kullan�l�yor.
			j=0;
			while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0 && j < 10000) {
				//Yeniden kesme �a�r�s� olu�umunu engellemek ve senkronizasyon sa�lamak
				//i�in timer s�f�rlan�yor.
				TIM4->CNT = 0;
				j++;
			}
			//��lem zaman a��m�na u�ramadan biter ise bir sonraki a�amaya ge�iliyor.
			if (j < 10000) {
				receiver_stage = SNC_RECEIVE_DATA;
				bit_counter=0;
			} else {
				//��lem zaman a��m�na u�rad� program resetleniyor.
				receiver_stage = START_POINT;
				err++;
			}
			return;
		}
		break;
	case SNC_RECEIVE_DATA:
		//Ba�lik ile adres verisi arasinda 12 bitlik bosluk var bunu atliyoruz
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
			//8bit adres 8bit komut okunuyor manchester kodlama oldu�u icin
			//1 bit iki a�amadan olu�uyor toplamda 16*2 32 bit.
		    //0 dan 1 e ge�i� 0, 1 den 0 a ge�i� 1 olarak de�erlendiriliyor.
		    //1 den 1 e ya da 0 dan 0 a ge�i� var ise hatal� giri� oldu�u i�in
		    //hata sayac� bir artt�r�l�yor ve d�ng�den ��k�l�yor.

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

        //Standart RC6 protokole g�re bu verinin command k�sm�nda yer almas� gerekir
		//As�l ihtiyac�m�z olan veri bu alanda bulunuyor 52-60 aral��� 8bit
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

	//IR al�c�n�n ba�l� oldu�u B portunun 0. pinini giri� olarak yap�land�r�yoruz.
	//E�er IR al�c� ba�ka bir porta ba�l� ise buradaki port numaras�n�
	//ve kesme fonksiyonu i�inde okuma yap�lan portu de�i�tirin.

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//Timer 444 mikrosaniyede bir kesme �retecek �ekilde yap�land�r�l�yoruz.
	//Ayr�ca senkronizasyon sa�lamak amac� ile kesme fonksiyonu i�erisinde timer s�f�rlan�yor.
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_InitStructure.TIM_Prescaler = 72;
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStructure.TIM_Period = 444;
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4, &TIM_InitStructure);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);


	//Kesme fonksiyonu ayarlan�yor
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM4, ENABLE);

	//De�i�kenlere ba�lang�� de�erleri veriliyor.
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

