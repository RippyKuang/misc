#include "stm32f10x.h"
#include "softTIM.h"
#include "USART.h"
#include "getVolt.h"
#include "Capture.h"
#include "stdio.h"



void delay( volatile int a){
	while(a--){
		 volatile int c=1200;
		while(c--);
	}
}

int main() 
{   
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	  GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
	    GPIO_InitTypeDef gpio;
     gpio.GPIO_Pin=	GPIO_Pin_11;
	 gpio.GPIO_Mode=GPIO_Mode_IPD;
	   GPIO_Init(GPIOE, &gpio);
   setUSART();
	getVolt_Init();
	OCStruct oc;
	oc.Polarity=Polarity_High;
	oc.pulse=3552;
	oc.OutputState=ENABLE;
	oc.OutputNState=DISABLE;
	 channel ch1;
	 ch1.pin=GPIO_Pin_10;
	 ch1.port=GPIOE; 
	 ch1.oc=&oc;
	softTIM_Init(72);
  softTIM tim;
  tim.arr=5000;
  tim.psc=1;
  tim.status=ENABLE;
  tim.ch[0]=&ch1;

  ICStruct ic;
  ic.filter=24;
  ic.CapState=DISABLE;
  ic.Polarity=Raising;
      channel ch2;
ch2.ic=&ic;
ch2.pin=GPIO_Pin_11;
ch2.port=GPIOE;
    softTIM tim2;
tim2.arr=500;
tim2.ch[0]=&ch2;
tim2.psc=1;
tim2.hander=Cap;
tim2.status=ENABLE;
  addgroup(&tim);
  addgroup(&tim2);


 while (1){
	ic.CapState=ENABLE;
	while(ic.CapState==ENABLE){}
unsigned	int a=ic.Updatetimes;
unsigned	int b=ic.cnt;
	float res1 =(a*500+b)*0.0001;
    ic.Polarity=Falling;
	ic.CapState=ENABLE;
	while(ic.CapState==ENABLE){}
unsigned	int c=ic.Updatetimes;
unsigned	int d=ic.cnt;
	float res2 =(c*500+d)*0.0001;

	printf("%f\n",res1/(res1+res2));
	delay(50);
}
	
}
