#ifndef __softTIM
#define __softTIM
#include "stm32f10x.h"
#define hardTIMx TIM6
#define hardTIMx_IRQHandler TIM6_IRQHandler
#define hardTIMx_IRQn TIM6_IRQn
#define hardTIMX_RCC RCC_APB1Periph_TIM6

#define Polarity_High 1
#define Polarity_Low 0
#define Raising 1
#define Falling 0
#define UpdateIRQ 0
#define CaptureIRQ 1

typedef struct soft_TIM softTIM;
typedef struct _channel channel;
typedef void (*softTIM_Hander)(char);
void softTIM_Init(unsigned int);
void addgroup(softTIM *);
void Cap(char);

typedef struct OC
{
    char Polarity;
    char OutputState;
    char OutputNState;
    channel* Nchan;
    unsigned int pulse;
} OCStruct;

typedef struct IC
{
     char Polarity;
     char filter;
 volatile  unsigned int cnt;
     char trigTimes;
     char isFirstCap;
  unsigned   int Updatetimes;
     char CapState;
     char step;
}ICStruct;

typedef struct _channel
{
    OCStruct *oc;
    ICStruct *ic;
    GPIO_TypeDef *port;
    unsigned short pin;
} channel;

typedef struct soft_TIM
{
    unsigned int arr;
   volatile unsigned int cnt;
    unsigned int psc;
    unsigned int trigTimes;
    char status;
    channel* ch[4];
    softTIM_Hander hander;
} softTIM;

#endif
