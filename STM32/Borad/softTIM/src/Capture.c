#include "Capture.h"
#include "stm32f10x.h"
   volatile char isCap=0;
   volatile  char isFirstCap=0;
   volatile int cnt=0;
   volatile  int time=0;
	 volatile static char mode;
	
char ifCapSucess(){
  return isCap;
}
double getCapRes(){
	 TIM_DeInit(TIM2);
	 TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_CC1, DISABLE);
   int temp1=cnt;
   int temp2=time;
   isCap=0;
   isFirstCap=0;
   cnt=0;
   time=0;
return (temp1+temp2*100)*0.0001;

}
void InitCap(){
	   GPIO_InitTypeDef gpio1;
	  gpio1.GPIO_Mode=GPIO_Mode_IPD;
	  gpio1.GPIO_Pin=GPIO_Pin_15;
	  GPIO_Init(GPIOA, &gpio1);
	  GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	  TIM_TimeBaseInitTypeDef tim2;
    tim2.TIM_ClockDivision = 0;
    tim2.TIM_Period = 100 - 1;
    tim2.TIM_Prescaler = 7200 - 1;
    tim2.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim2);

    TIM_ICInitTypeDef cap;
    cap.TIM_Channel = TIM_Channel_1;
    cap.TIM_ICFilter = 0x0;
    cap.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    cap.TIM_ICSelection = TIM_ICSelection_DirectTI;
	if(mode==1)
    cap.TIM_ICPolarity = TIM_ICPolarity_Rising;
	else
		 cap.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInit(TIM2, &cap);

    NVIC_InitTypeDef NVIC_cap;
    NVIC_cap.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_cap.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_cap.NVIC_IRQChannelSubPriority = 0;
    NVIC_cap.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_cap);
    TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_CC1, ENABLE);
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update | TIM_IT_CC1);
		TIM_Cmd(TIM2, ENABLE);
	
}
void startCap(char lmode){
	 isCap=0;
   isFirstCap=0;
   cnt=0;
   time=0;
	 mode=lmode;
	 InitCap();
}
void TIM2_IRQHandler(void)
{ 
  if(isCap==0){
        if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
        {   
         if(isFirstCap!=0)					
            time++;     
        }
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
        {   
              if(isFirstCap==0){
                    isFirstCap=1;
                     TIM_SetCounter(TIM2,0);
                  if(mode==1)    TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Falling);
								else   TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Rising);
               }else{
                     cnt=TIM_GetCapture1(TIM2);
                     isCap=1;
               }                                                
         }

    }
	  TIM_ClearITPendingBit(TIM2, TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
	 }
