#include "USART.h"
#include "stm32f10x.h"
#include "stdio.h"


int fputc(int ch, FILE *p)
{
    USART_SendData(UART4, (u8)ch);
    while (USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET) ; 
    return ch;
}
void setUSART()
{   
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitTypeDef u;
    u.USART_BaudRate = 115200;
    u.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    u.USART_Mode = USART_Mode_Tx;
    u.USART_Parity = USART_Parity_No;
    u.USART_StopBits = USART_StopBits_1;
    u.USART_WordLength = USART_WordLength_8b;
    USART_Init(UART4, &u);
    USART_Cmd(UART4, ENABLE);
}
