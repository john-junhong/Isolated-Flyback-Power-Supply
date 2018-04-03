#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "adc.h"
#include "control.h"
#include "lcd.h"
#include "pid.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//�ⲿ�ж� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
extern u8 choose,Update,SetFreq;	
extern HEDIT *edit1,*edit2,*edit3,*edit4,*edit5,*edit6;
extern float pwm;
extern float SetVol;
void EXTI_GPIO_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//??GPIOA,GPIOE??
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //KEY0 KEY1 KEY2????
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//??????
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//??
	GPIO_Init(GPIOE, &GPIO_InitStructure);//???GPIOE4
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //KEY0 KEY1 KEY2????
	GPIO_Init(GPIOE, &GPIO_InitStructure);//???GPIOE4
}
void EXTI3_IRQHandler(void)//�����жϣ���������������
{
	u16 adc;
	double Current;
	adc=Get_Adc_Average(ADC1,ADC_Channel_2,30);
	Current=(adc/4096.0*2.5)/0.5;
	strcpy((char*)edit2->text,(char*)RealToStr(Current));
	Update=1;
	EXTI_ClearITPendingBit(EXTI_Line3);//??LINE2??????? 
}
void EXTI2_IRQHandler(void)//�����жϣ�����ѹ��������
{
	/*
	adc=Get_Adc_Average(ADC1,ADC_Channel_2,30);
	Voltage=(adc/4096.0*2.5)*30;
	if (choose==1) PID(Voltage,36,&pwm);
	update_time++;-
	if (update_time>SetFreq/2) {
		update_time=0;
		strcpy((char*)edit2->text,(char*)RealToStr(Voltage));
		Update=1;
	}*/
	LED0=!LED0;
	EXTI_ClearITPendingBit(EXTI_Line2);  //??LINE3???????  
}
	   
//�ⲿ�жϳ�ʼ������
//��ʼ��PE2~4,PA0Ϊ�ж�����.
void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	EXTI_GPIO_Init(); //������Ӧ��IO�ڳ�ʼ��
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//ʹ��SYSCFGʱ��
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);
	
	
	/* ����EXTI_Line2,3,4 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//�ж���ʹ��
	EXTI_Init(&EXTI_InitStructure);//����
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش���
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;//�ж���ʹ��
	EXTI_Init(&EXTI_InitStructure);//����
 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;//�ⲿ�ж�0
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);//����
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;//�ⲿ�ж�2
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);//����
	   
}












