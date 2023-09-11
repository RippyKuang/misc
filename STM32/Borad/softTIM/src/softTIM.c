#include "softTIM.h"
#include "stdlib.h"

softTIM *timgroup[10];

char NofTIM = 0;
volatile char current_TIM;
ICStruct *tempIC;
void hardTIMx_IRQHandler()
{
    if (TIM_GetITStatus(hardTIMx, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(hardTIMx, TIM_IT_Update);
        for (char a = 0; a < NofTIM; a++)
        {
            current_TIM = a;
            if (timgroup[a]->status != DISABLE)
            {
                timgroup[a]->trigTimes++;
							 for (int b = 0; b < 4; b++)
                {
                    if (timgroup[a]->ch[b] != NULL)
                    {
                        if (timgroup[a]->ch[b]->ic != NULL)
                        {
                            if (timgroup[a]->ch[b]->ic->CapState != DISABLE)
                            {
                                tempIC = timgroup[a]->ch[b]->ic;
                                channel *tempCHAN = timgroup[a]->ch[b];
                                char level = GPIO_ReadInputDataBit(tempCHAN->port, tempCHAN->pin);
                                if (tempIC->Polarity == Raising)
                                {
                                    if (level == 0)
                                    {
                                        tempIC->step = 1;
                                    }
                                    else
                                    {
                                        if (level == 1 && tempIC->step == 1)
                                        {
                                            tempIC->trigTimes++;
                                            if (tempIC->trigTimes >= tempIC->filter)
                                            {
                                                tempIC->trigTimes = 0;
                                                tempIC->step = 0;
                                                timgroup[a]->hander(CaptureIRQ);
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (level == 1)
                                    {
                                        tempIC->step = 1;
                                    }
                                    else
                                    {
                                        if (level == 0 && tempIC->step == 1)
                                        {
                                            tempIC->trigTimes++;
                                            if (tempIC->trigTimes >= tempIC->filter)
                                            {
                                                tempIC->trigTimes = 0;
                                                tempIC->step = 0;
                                                timgroup[a]->hander(CaptureIRQ);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (timgroup[a]->trigTimes == timgroup[a]->psc)
                {
                    timgroup[a]->trigTimes = 0;
                    timgroup[a]->cnt++;
                    for (int b = 0; b < 4; b++)
                    {
                        if (timgroup[a]->ch[b] != NULL)
                        {
                            if (timgroup[a]->ch[b]->oc != NULL)
                            {
                                if (timgroup[a]->ch[b]->oc->OutputState == ENABLE)
                                {
                                    OCStruct *tempOC = timgroup[a]->ch[b]->oc;
                                    channel *tempCHAN = timgroup[a]->ch[b];
                                    if (tempOC->Polarity == Polarity_High)
                                    {
                                        char flag;
                                        if (timgroup[a]->cnt > tempOC->pulse)
                                        {
                                            GPIO_SetBits(tempCHAN->port, tempCHAN->pin);
                                            flag = 1;
                                        }
                                        else
                                        {
                                            GPIO_ResetBits(tempCHAN->port, tempCHAN->pin);
                                            flag = 0;
                                        }
                                        if (tempOC->OutputNState == ENABLE)
                                        {
                                            tempCHAN = tempOC->Nchan;
                                            if (flag == 1)
                                                GPIO_ResetBits(tempCHAN->port, tempCHAN->pin);
                                            else
                                                GPIO_SetBits(tempCHAN->port, tempCHAN->pin);
                                        }
                                    }
                                    else
                                    {
                                        char flag;
                                        if (timgroup[a]->cnt > tempOC->pulse)
                                        {
                                            flag = 1;
                                            GPIO_ResetBits(tempCHAN->port, tempCHAN->pin);
                                        }
                                        else
                                        {
                                            flag = 0;
                                            GPIO_SetBits(tempCHAN->port, tempCHAN->pin);
                                        }
                                        if (tempOC->OutputNState == ENABLE)
                                        {
                                            tempCHAN = tempOC->Nchan;
                                            if (flag == 1)
                                                GPIO_SetBits(tempCHAN->port, tempCHAN->pin);
                                            else
                                                GPIO_ResetBits(tempCHAN->port, tempCHAN->pin);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (timgroup[a]->cnt == timgroup[a]->arr)
                    {
                        timgroup[a]->cnt = 0;

                        if (timgroup[a]->hander != NULL)
                            timgroup[a]->hander(UpdateIRQ);
                    }
                }
               
            }
        }
    }
}
void addgroup(softTIM *stim)
{
    if (NofTIM < 10)
    {
        stim->trigTimes = 0;
        timgroup[NofTIM] = stim;
        NofTIM++;
    }
}

void softTIM_Init(unsigned int psc)
{
    RCC_APB1PeriphClockCmd(hardTIMX_RCC, ENABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitTypeDef timint;
    timint.NVIC_IRQChannel = hardTIMx_IRQn;
    timint.NVIC_IRQChannelPreemptionPriority = 1;
    timint.NVIC_IRQChannelSubPriority = 0;
    timint.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&timint);

    TIM_TimeBaseInitTypeDef timbase;
    timbase.TIM_Prescaler = psc - 1;
    timbase.TIM_Period = 100 - 1;
    timbase.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(hardTIMx, &timbase);
    TIM_ITConfig(hardTIMx, TIM_IT_Update, ENABLE);
    TIM_Cmd(hardTIMx, ENABLE);
}

void Cap(char a)
{

    if (a == UpdateIRQ)
    {
        if (tempIC->isFirstCap != 0)
            tempIC->Updatetimes++;
    }
    if (a == CaptureIRQ) 
    {
        if (tempIC->isFirstCap == 0)
        {
            tempIC->isFirstCap = 1;
            timgroup[current_TIM]->cnt = 0;
					 tempIC->Updatetimes = 0;
            if (tempIC->Polarity == Raising)
                tempIC->Polarity = Falling;
            else
                tempIC->Polarity = Raising;
        }
        else
        {
            tempIC->cnt = timgroup[current_TIM]->cnt;
            tempIC->CapState = DISABLE;
            tempIC->isFirstCap = 0;
           
        }
    }
}
