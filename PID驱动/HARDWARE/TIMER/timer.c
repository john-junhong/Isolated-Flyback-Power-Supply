#include "timer.h"
#include "pwm.h"
#include "sys.h"	
#include "lcd.h"
#include "pid.h"
#include <string.h>
#include "control.h"
#include "adc.h"
#include "math.h"
#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 


extern u16 chart[200];
extern float pwm;
extern u8 nowpwm,POWER;
extern float setValue;
extern u8 choose,Update;
extern float SetFreq;	
extern u16 delay,delay_left;
extern float SetVol,integral;
//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 period)//TIM3 ��ʱ����Ƶ42MHz
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = period-1; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=0;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=0; 
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

u16 adc0,adc1,adc2,adc3,POWERTimer;
float volt1,volt2,amp1,amp2;
extern HEDIT *edit1,*edit2,*edit3,*edit4,*edit5,*edit6;
u8 nowEXTI2,lastEXTI2,nowEXTI3,lastEXTI3;
static u16 update_time=0;
double Voltage,VoltageIn,Current;
//��ʱ��3�жϷ����� 10kHz
void TIM3_IRQHandler(void)
{
	u16 adc;
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		if (POWER) {
			adc=Get_Adc_Average(ADC1,ADC_Channel_1,30);
			Voltage=(adc/4096.0*3.269)*3.3386+28.314;
			if (choose==1) PID(Voltage,SetVol,&pwm); //CloseLoop
		
			adc=Get_Adc_Average(ADC1,ADC_Channel_0,30);
			VoltageIn=(adc/4096.0*3.269)*3.3806+16.054;
	
			adc=Get_Adc_Average(ADC1,ADC_Channel_3,30);
			Current=(adc/4096.0*3.269)*1.7657-2.2301;
			if (Current>1.539) {  
				POWERTimer=0;
				POWER=0;
				pwm=0;
				//��������
			}
		} else {
			POWERTimer++;
			Voltage=0; VoltageIn=0; Current=0;
			if (POWERTimer>10000) {
				POWER=1; integral=0;
			}
		}
		
		update_time++;
		if (update_time>2000) {	//Update Frequency 5Hz
			update_time=0;
			strcpy(edit1->text,RealToStr(Voltage));
			strcpy(edit2->text,RealToStr(Current));
			strcpy(edit3->text,RealToStr(VoltageIn));
			strcpy(edit4->text,RealToStr(Round(pwm*100.0))); 
			Update=1;
		}
		TIM_SetCompare1(TIM1,Round(1680*pwm));
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}
