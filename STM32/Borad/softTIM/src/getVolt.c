#include "getVolt.h"
#include "stm32f10x.h"
volatile uint16_t data[1];
void setDMA(){
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)((&ADC1->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc =DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1,ENABLE);
}
void setADC(){  
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Mode=GPIO_Mode_AIN; 
    gpio.GPIO_Pin=GPIO_Pin_6;
     GPIO_Init(GPIOA,&gpio);
	ADC_InitTypeDef adc;
	adc.ADC_Mode=ADC_Mode_Independent ;
	adc.ADC_ScanConvMode=DISABLE;
	adc.ADC_ContinuousConvMode=ENABLE;
	adc.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
	adc.ADC_DataAlign=ADC_DataAlign_Right;
	adc.ADC_NbrOfChannel=1;
	ADC_Init(ADC1, &adc);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_239Cycles5);
	ADC_Cmd(ADC1,ENABLE);
   ADC_DMACmd(ADC1,ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1)){};
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1)){};
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
void getVolt_Init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	setDMA();
	setADC();
}
float getVolt(){
    return (float)data[0] / 4096.0f * 3.3f * 1.5f ;
}

