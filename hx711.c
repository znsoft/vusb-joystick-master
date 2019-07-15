#include <avr/io.h>

#define HX_PORT PORTB
#define HX_DDR  DDRB
#define HX_PIN  PINB

#define data 0
#define sck 1
//результат взвешивания
int32_t adc_value = 0;

void HX711_Init(void)
{   
	//sck выход, data - вход
	HX_DDR |= (1<<sck);
	HX_DDR &= ~(1<<data);
	
	//подтягиваем data к питанию, на sck устанавливаем лог.единицу
	HX_PORT |= (1<<data);
	HX_PORT &= ~(1<<sck);
}

uint32_t Weighing(void)
{
	uint8_t i = 0;
	adc_value = 0;

	HX_PORT &= ~(1<<sck);
	//ждём пока закончится преобразование
	while((HX_PIN & (1<<data)));
	
	for(i=0; i<24; i++)
	{
		//поднимаем строб
		HX_PORT |= (1<<sck);
		//сдвигаем значение АЦП влево, теперь значение АЦП выглядит так .......0
		adc_value <<= 1;
		//опускаем строб
		HX_PORT &= ~(1<<sck);
		//проверяем, что на выводе data, если ноль то самый правый символ так и останется ноль .......0
		//если 1, то самый правый символ будет один .......1
		if(HX_PIN & (1<<data))
		{
			adc_value++;
		}	
	}

	//выборки будем брать с канала А, с коэф.усиления 128
	//поэтому стробируем еще один раз
	HX_PORT |= (1<<sck);
	HX_PORT &= ~(1<<sck);
	
	return adc_value;
}
/*
int main(void)
{

    HX711_Init();

    while(1)
    {
          Weighing(); 
    }
}*/