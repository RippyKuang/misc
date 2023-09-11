
#include "stm32f10x.h"
#include "include.h"
#include "mpu6050.h"
#include "stdio.h"
#include "math.h"
#include "USART.h"

static void delay_ms( volatile int a){
	while(a--){
		 volatile int c=1200;
		while(c--);
	}
}
__IO float axis_x,axis_y,axis_z;

void TIM3_IRQHandler(){
	 if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){
		 float _axis_x,_axis_y,_axis_z;
		 float _x,_y,_z;
		  MPU_Get_Gyroscope(&_axis_x,&_axis_y,&_axis_z);
		 MPU_Get_Accelerometer(&_x,&_y,&_z);

		 axis_y=0.8*( _axis_y*0.001+axis_y)-0.2*atan(_x/sqrt(_y*_y+_z*_z))*57.3;
		 axis_x=0.8*(_axis_x*0.001+axis_x)+0.2*atan(_y/_z)*57.3;
	    
		axis_y=_y;
		axis_x=_x;
    
		  TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	 }
 }
int main()
{     MPU_Init();
      setUSART();
      calibration();
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 ,ENABLE);
	NVIC_InitTypeDef tim3;
	tim3.NVIC_IRQChannel=TIM3_IRQn;
	tim3.NVIC_IRQChannelPreemptionPriority=0 ;
	tim3.NVIC_IRQChannelSubPriority=0;
	tim3.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&tim3);
      TIM_TimeBaseInitTypeDef tim;
      tim.TIM_ClockDivision = 0;
      tim.TIM_CounterMode = TIM_CounterMode_Up;
      tim.TIM_Period = 1000;
      tim.TIM_Prescaler = 71;
      TIM_TimeBaseInit(TIM3, &tim);
      TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
      TIM_Cmd(TIM3, ENABLE);	
  while (1){
		delay_ms(1);
		printf("%lf,%lf\n",axis_x,axis_y);
	   };
}
