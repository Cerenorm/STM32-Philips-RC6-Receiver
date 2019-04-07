#ifndef RC6_H
#define RC6_H

/*
 * 30-03-2019
 * notpast.com
 * Bu program RC6 tipi uzaktan kumanda sinyalini almak için yazýlmýþtýr.
 * Donaným olarak IR alýcý, OLED ekran, STM32f103c8 MCU, HP RC6 uzaktan kumanda kullanýlmýþtýr.
 * Farklý  MCU lara da adabte edilebilir.
 * Philips RC6 protokol baz alýnarak sinyalin bir kýsmý çözümlenmiþtir geri kalaný ise
 * sinyal analiz edilerek bilenþenlerine ayrýlmýþtýr.
 * Sinyalin bit geniþliði 444 mikrosaniyedir. Bu yüzden TIM4 444 mikrosaniyede
 * bir kesme üretecek þekilde ayarlanmýþtýr. Kesme fonksiyonunda ilk baþta
 * IR sensörden gelen veri okunuyor ve aþamalý olarak baþlýk,adres,komut,veri
 * bileþenleri olarak kaydediliyor. Tüm iþlemler kesme fonksiyonu içerisinde yapýlýyor.
 * Program çýktýlarý:
 * 			addrees: Sinyalden okunan adres verisi (1 bayt).
 * 			command: Sinyalden okunan komut verisi (1 bayt).
 * 			remote_data: Sinyalden okunan veri, basýlan tuþa ait kod (1 bayt).
 * 			err: Sinyal okuma sýrasýnda oluþan hata sayýsý.
 * 			led: Baslýk okunduðu zaman tersi alýnan bir deðiþken.
 * 			     Led baðlý bir çýkýþa baðlandýðýnda sinyal baþlýðý okunduðu zaman
 * 			     led yanýp söner.
 */



#define START_POINT 		0
#define READ_FIRST_BIT 		1
#define SNC_RECEIVE_DATA 	2
#define READY_TO_READ		3



//Çýkýþ deðiþkenleri
volatile int led ;
volatile int err;
volatile unsigned int command;
volatile unsigned int address;
volatile int remote_data;
volatile int receiver_stage;




//Ýlk deðerleri veren ve timer ve kesme fonksiyonunu ayarlayan fonksiyon.
void rc6Init(void);


#endif
