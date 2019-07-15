#include <avr/io.h>

#define HX_PORT PORTB
#define HX_DDR  DDRB
#define HX_PIN  PINB

#define data 0
#define sck 1
//��������� �����������
int32_t adc_value = 0;

void HX711_Init(void)
{   
	//sck �����, data - ����
	HX_DDR |= (1<<sck);
	HX_DDR &= ~(1<<data);
	
	//����������� data � �������, �� sck ������������� ���.�������
	HX_PORT |= (1<<data);
	HX_PORT &= ~(1<<sck);
}

uint32_t Weighing(void)
{
	uint8_t i = 0;
	adc_value = 0;

	HX_PORT &= ~(1<<sck);
	//��� ���� ���������� ��������������
	while((HX_PIN & (1<<data)));
	
	for(i=0; i<24; i++)
	{
		//��������� �����
		HX_PORT |= (1<<sck);
		//�������� �������� ��� �����, ������ �������� ��� �������� ��� .......0
		adc_value <<= 1;
		//�������� �����
		HX_PORT &= ~(1<<sck);
		//���������, ��� �� ������ data, ���� ���� �� ����� ������ ������ ��� � ��������� ���� .......0
		//���� 1, �� ����� ������ ������ ����� ���� .......1
		if(HX_PIN & (1<<data))
		{
			adc_value++;
		}	
	}

	//������� ����� ����� � ������ �, � ����.�������� 128
	//������� ���������� ��� ���� ���
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