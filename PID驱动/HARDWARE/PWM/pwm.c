/******************** (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� ********************
 * �ļ���  ��pwm_output.c
 * ����    ��         
 * ʵ��ƽ̨��Ұ��STM32������
 * Ӳ�����ӣ�---------------------
 *          |  PA.06: (TIM3_CH1)  |
 *          |  PA.07: (TIM3_CH2)  |
 *      	  |  PB.00: (TIM3_CH3)  | 
 *    		  |  PB.01: (TIM3_CH4)  |
 *           ---------------------    			
 * ��汾  ��ST3.0.0
 *
 * ����    ��fire  QQ: 313303034 
 * ����    ��firestm32.blog.chinaunix.net
**********************************************************************************/
#include "pwm.h"
#include "sys.h"	
#include "lcd.h"
#include "pid.h"
#include <string.h>
#include "control.h"
#include "adc.h"
#include "math.h"
#include "led.h"
#include "key.h"

extern float setValue;
extern u8 choose,Update;
extern float SetFreq;	
extern u16 delay,delay_left;
extern float pwm;
extern float SetVol;
/*
 * ��������TIM3_GPIO_Config
 * ����  ������TIM3�������PWMʱ�õ���I/O
 * ����  ����
 * ���  ����
 * ����  ���ڲ�����
 */
static void TIM1_GPIO_Config(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;

	/* TIM3 clock enable */
	//PCLK1����2��Ƶ����ΪTIM3��ʱ��Դ����84MHz

  /* GPIOA and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE); 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);

  GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_TIM1); //��һ��ͨ��ռ�ձ�pwm
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_TIM1);//��һ��ͨ��ռ�ձ�1-pwm
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_TIM1);//�ڶ���ͨ��
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_TIM1);
  /*GPIOA Configuration: TIM3 channel 1 and 2 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //??????
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;	// �����������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_14;
  GPIO_Init(GPIOB, &GPIO_InitStructure);


  /*GPIOB Configuration: TIM3 channel 3 and 4 as alternate function push-pull */
  //GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;

  //GPIO_Init(GPIOB, &GPIO_InitStructure);
}


void TIM1_Mode_Config(u16 period)
{      
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_BDTRInitTypeDef TIM1_BDTRInitStructure;
	
	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = period-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 4;//4�����ڲŽ����ж�

	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* Channel 1, 2,3 and 4 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

	/* 25% duty */
	//TIM_OCInitStructure.TIM_Pulse = ;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);           /* Set OC1&OC1N duty */
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //??TIM14?CCR1????????
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);           /* Set OC1&OC1N duty */
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
 
	TIM_ARRPreloadConfig(TIM1,ENABLE);//ARPE?? 
	/* Automatic Output enable, Break, dead time and lock configuration*/
    TIM1_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
    TIM1_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
    TIM1_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1; 
    TIM1_BDTRInitStructure.TIM_DeadTime = 0x1C;        /* 1us */
    TIM1_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
    TIM1_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;
    TIM1_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
    TIM_BDTRConfig(TIM1,&TIM1_BDTRInitStructure);


	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM1_UP_TIM10_IRQn; //???3??
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //?????1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; //????3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearFlag(TIM1,TIM_FLAG_Update);   //????????????????
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	//TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	
	/* TIM1 counter enable */
	TIM_Cmd(TIM1, ENABLE);
}
void TIM1_PWM_Init(void)
{
	TIM1_GPIO_Config();
	TIM1_Mode_Config(840*2);	
	//TIM_SetCompare1(TIM1,420*2);
}
void On_PWM(void)
{
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}
void Off_PWM(void)
{
	TIM_CtrlPWMOutputs(TIM1, DISABLE);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);
}

void TIM1_UP_TIM10_IRQHandler(void)//ÿ���������ڵ��ж�
{
	if(TIM_GetITStatus(TIM1,TIM_FLAG_Update)==SET) //????
	{
		TIM_ClearITPendingBit(TIM1,TIM_FLAG_Update); 
		

		//time++;
		/*if (time>=10000) {
			time=0;
			adc0=Get_Adc_Average(ADC1,ADC_Channel_0,30);
			adc1=Get_Adc_Average(ADC1,ADC_Channel_1,15);
			adc2=Get_Adc_Average(ADC1,ADC_Channel_2,30);
			//adc3=Get_Adc_Average(ADC_Channel_3,15);;
			volt1=(adc0*2.4945/4096)*4+5;
			amp1=(1.249-adc1*2.4945/4096)/50/0.006;
			volt2=(adc2*2.4945 /4096)*4+20;
			amp2=0;
		
			strcpy(edit1->text,RealToStr(volt1));
			strcpy(edit2->text,RealToStr(amp1)); 
			strcpy(edit3->text,RealToStr(volt2));
			strcpy(edit4->text,RealToStr(amp2)); 
			Update=1;
			
			time2++;
			if (time2==3) {
				time2=0;
				if (choose==2) {
					if (volt2<22.5) setValue=-3;
					else if (volt2>23.5) setValue=3;
					else {
						setValue=-3+6*(volt2-22.5);
					}
					if (fabs(volt1-15)<0.25) setValue=0.05;
					strcpy(edit6->text,RealToStr(setValue)); UpdateWindow(edit6);
				}
			}
			if (volt2>28) {Off_PWM();delay=1; delay_left=10000;} else On_PWM();
			if (volt1<7&&choose==1) {Off_PWM();delay=1; delay_left=10000;} else On_PWM();
		}
		if (delay) {
			Off_PWM();
			delay_left--;
			if (delay_left==0) {
				delay=0; On_PWM();
			}
		}
		if (!delay) {
			adc1=Get_Adc_Average(ADC1,ADC_Channel_1,5);
			amp1=(1.249-adc1*2.5/4096)/50/0.006;
			PID(amp1,setValue,&dutyCycle);
			TIM_SetCompare1(TIM1,(u16)(dutyCycle+0.5));
		}*/
	} else TIM_ClearITPendingBit(TIM1,TIM_FLAG_Update);
}
/******************* (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� *****END OF FILE*****/
