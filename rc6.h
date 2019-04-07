#ifndef RC6_H
#define RC6_H

/*
 * 30-03-2019
 * notpast.com
 * Bu program RC6 tipi uzaktan kumanda sinyalini almak i�in yaz�lm��t�r.
 * Donan�m olarak IR al�c�, OLED ekran, STM32f103c8 MCU, HP RC6 uzaktan kumanda kullan�lm��t�r.
 * Farkl�  MCU lara da adabte edilebilir.
 * Philips RC6 protokol baz al�narak sinyalin bir k�sm� ��z�mlenmi�tir geri kalan� ise
 * sinyal analiz edilerek bilen�enlerine ayr�lm��t�r.
 * Sinyalin bit geni�li�i 444 mikrosaniyedir. Bu y�zden TIM4 444 mikrosaniyede
 * bir kesme �retecek �ekilde ayarlanm��t�r. Kesme fonksiyonunda ilk ba�ta
 * IR sens�rden gelen veri okunuyor ve a�amal� olarak ba�l�k,adres,komut,veri
 * bile�enleri olarak kaydediliyor. T�m i�lemler kesme fonksiyonu i�erisinde yap�l�yor.
 * Program ��kt�lar�:
 * 			addrees: Sinyalden okunan adres verisi (1 bayt).
 * 			command: Sinyalden okunan komut verisi (1 bayt).
 * 			remote_data: Sinyalden okunan veri, bas�lan tu�a ait kod (1 bayt).
 * 			err: Sinyal okuma s�ras�nda olu�an hata say�s�.
 * 			led: Basl�k okundu�u zaman tersi al�nan bir de�i�ken.
 * 			     Led ba�l� bir ��k��a ba�land���nda sinyal ba�l��� okundu�u zaman
 * 			     led yan�p s�ner.
 */



#define START_POINT 		0
#define READ_FIRST_BIT 		1
#define SNC_RECEIVE_DATA 	2
#define READY_TO_READ		3



//��k�� de�i�kenleri
volatile int led ;
volatile int err;
volatile unsigned int command;
volatile unsigned int address;
volatile int remote_data;
volatile int receiver_stage;




//�lk de�erleri veren ve timer ve kesme fonksiyonunu ayarlayan fonksiyon.
void rc6Init(void);


#endif
