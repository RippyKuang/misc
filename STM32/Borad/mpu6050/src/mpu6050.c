#include "mpu6050.h"
#include "stm32f10x.h"



LowPassFilter lpf_x;
LowPassFilter lpf_y;
LowPassFilter lpf_z;
float noise_x;
float noise_y;
float noise_z;


 float LPFilter(LowPassFilter* f,float x){
   f->lasty=0.2*x+0.8*f->lasty;
   return f->lasty;
}
 
static void delay_ms( volatile int a){
	while(a--){
		 volatile int c=1200;
		while(c--);
	}
}

static void i2c_delay()
{
	__IO uint8_t i;
	for(i=0;i<10;i++){
	}
}


void i2c_Start(void){
    mpu_I2C_SDA_1();      
    mpu_I2C_SCL_1();
    i2c_delay();   
	
    mpu_I2C_SDA_0();
    i2c_delay();
	
    mpu_I2C_SCL_0();
    i2c_delay();
}

void i2c_Stop(void){
    mpu_I2C_SDA_0();       
    mpu_I2C_SCL_1();
    i2c_delay();         
    mpu_I2C_SDA_1();
}

uint8_t i2c_WaitAck(void){   //0表示正确应答,1表示无应答
    uint8_t r;
    mpu_I2C_SDA_1();        
    i2c_delay();        
    mpu_I2C_SCL_1();        
    i2c_delay();
    if(mpu_I2C_SDA_READ())
        r = 1; 
    else
        r = 0;   
    mpu_I2C_SCL_0();
    i2c_delay();
    return r;
}
void i2c_Ack(){
	  mpu_I2C_SDA_0();
  	i2c_delay();
	  mpu_I2C_SCL_1();
	  i2c_delay();
	  mpu_I2C_SCL_0();
	  i2c_delay();
	  mpu_I2C_SDA_1();
}

void i2c_NAck(void){
    mpu_I2C_SDA_1();    
    i2c_delay();
    mpu_I2C_SCL_1();    
    i2c_delay();
    mpu_I2C_SCL_0();
    i2c_delay();
}


void i2c_SendByte(uint8_t _ucByte){
    uint8_t i;
    for(i=0;i<8;i++)
    {
        if(_ucByte & 0x80)
            mpu_I2C_SDA_1();
        else
            mpu_I2C_SDA_0();
        i2c_delay();
          mpu_I2C_SCL_1();
        i2c_delay();
          mpu_I2C_SCL_0();
        if(i == 7)
           mpu_I2C_SDA_1();    
       _ucByte <<= 1;
        i2c_delay();
    }
}

uint8_t i2c_ReadByte(unsigned char ack){ //1发送ack 0发送nack
    uint8_t i;
    uint8_t value = 0;
    for(i = 0;i<8;i++)
    {
        value <<= 1;
        mpu_I2C_SCL_1();
        i2c_delay();
        if(mpu_I2C_SDA_READ())
        {
            value++;
        }
        mpu_I2C_SCL_0();
        i2c_delay();
    }
		if(ack==1) i2c_Ack();
		else i2c_NAck();
    return value;
}

//0成功 1失败
u8 MPU_Write_Byte(u8 reg,u8 data){ 
    i2c_Start(); 
	  i2c_SendByte((MPU_ADDR<<1)|0);//发送器件地址+写命令	
	if(i2c_WaitAck())	//等待应答
	{
		i2c_Stop();		 
		return 1;		
	}
    i2c_SendByte(reg);	//写寄存器地址
    i2c_WaitAck();		//等待应答 
	   i2c_SendByte(data);//发送数据
	if(i2c_WaitAck())	//等待ACK
	{
		i2c_Stop();	 
		return 1;		 
	}		 
   i2c_Stop();	 
	 return 0;
}


u8 MPU_Read_Byte(u8 reg)
{
	u8 res;
    i2c_Start(); 
	i2c_SendByte((MPU_ADDR<<1)|0);
	i2c_WaitAck();		//等待应答 
    i2c_SendByte(reg);	//写寄存器地址
    i2c_WaitAck();		//等待应答
    i2c_Start();
	i2c_SendByte((MPU_ADDR<<1)|1);//发送器件地址+读命令	
    i2c_WaitAck();		//等待应答 
	res=i2c_ReadByte(0);//读取数据,发送nACK 
    i2c_Stop();			//产生一个停止条件 
	return res;		
}


u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i; 
    i2c_Start(); 
	i2c_SendByte((addr<<1)|0);//发送器件地址+写命令	
	if(i2c_WaitAck())	//等待应答
	{
		i2c_Stop();		 
		return 1;		
	}
    i2c_SendByte(reg);	//写寄存器地址
    i2c_WaitAck();		//等待应答
	for(i=0;i<len;i++)
	{
		i2c_SendByte(buf[i]);	//发送数据
		if(i2c_WaitAck())		//等待ACK
		{
			i2c_Stop();	 
			return 1;		 
		}		
	}    
    	i2c_Stop();	 
	return 0;	
} 

u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{ 
 	i2c_Start(); 
	i2c_SendByte((addr<<1)|0);//发送器件地址+写命令	
	if(i2c_WaitAck())	//等待应答
	{
		i2c_Stop();		 
		return 1;		
	}
    i2c_SendByte(reg);	//写寄存器地址
    i2c_WaitAck();		//等待应答
    	i2c_Start(); 
	i2c_SendByte((addr<<1)|1);//发送器件地址+读命令	
     i2c_WaitAck();			//等待应答 
	while(len)
	{
		if(len==1)*buf=i2c_ReadByte(0);//读数据,发送nACK 
		else *buf=i2c_ReadByte(1);		//读数据,发送ACK  
		len--;
		buf++; 
	}    
   	i2c_Stop();	//产生一个停止条件 
	return 0;	
}

//得到加速度值
//gx,gy,gz:陀螺仪x,y,z轴经低通滤波后的读数(带符号)
//返回值:0,成功
//    其他,错误代码
u8 MPU_Get_Accelerometer(float *ax,float *ay,float *az)
{
    short rx,ry,rz;
	   u8 buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		rx=((u16)buf[0]<<8)|buf[1];  
		ry=((u16)buf[2]<<8)|buf[3];  
		rz=((u16)buf[4]<<8)|buf[5];
		
	float temp_x=(2*9.8*rx)/32768.0;
		float temp_y=(2*9.8*ry)/32768.0;
		 float temp_z=(2*9.8*rz)/32768.0;
	*ax=LPFilter(&lpf_x,temp_x);
	      *ay=LPFilter(&lpf_y,temp_y);
		  *az=LPFilter(&lpf_z,temp_z);	 
	} 	
    return res;;
}


//得到温度值
//返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(void)
{
    u8 buf[2]; 
    short raw;
	float temp;
	MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((u16)buf[0]<<8)|buf[1];  
    temp=36.53+((double)raw)/340.0;  
    return temp*100;;
}


//得到陀螺仪值
//gx,gy,gz:陀螺仪x,y,z轴的读数(带符号)
//返回值:0,成功
//    其他,错误代码
u8 MPU_Get_Gyroscope(float *gx,float *gy,float *gz)
{
	 short rx,ry,rz;
    u8 buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		rx=((u16)buf[0]<<8)|buf[1];  
		ry=((u16)buf[2]<<8)|buf[3];  
		rz=((u16)buf[4]<<8)|buf[5];
		
		float temp_x=(2000.0*rx)/32768.0;
		float temp_y=(2000.0*ry)/32768.0;
		float temp_z=(2000.0*rz)/32768.0;
		*gx=temp_x-noise_x;
		*gy=temp_y-noise_y;
		*gz=temp_z;
	} 	
    return res;;
}

//设置MPU6050陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG,fsr<<3);//设置陀螺仪满量程范围  
}


//设置MPU6050加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);//设置加速度传感器满量程范围  
}


//设置MPU6050的数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return MPU_Write_Byte(MPU_CFG_REG,data);//设置数字低通滤波器  
}



//设置MPU6050的采样率(假定Fs=1KHz)
//rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	//设置数字低通滤波器
 	return MPU_Set_LPF(rate/2);	//自动设置LPF为采样率的一半
}

 char MPU_Init(void){
	 lpf_z.lasty=9.8;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(mpu_RCC_I2C_PORT, ENABLE);	
	GPIO_InitStructure.GPIO_Pin = mpu_I2C_SCL_PIN | mpu_I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  	/* 开漏输出 */
	GPIO_Init(mpu_GPIO_PORT_I2C, &GPIO_InitStructure);
	i2c_Stop();
	
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//复位MPU6050
  delay_ms(100);
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//唤醒MPU6050 
	MPU_Set_Gyro_Fsr(3);					//陀螺仪传感器,±2000dps
	MPU_Set_Accel_Fsr(0);					//加速度传感器,±2g
	MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//关闭所有中断
	MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C主模式关闭
	MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);	//关闭FIFO
	MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);	//INT引脚低电平有效
u8 res=MPU_Read_Byte(MPU_DEVICE_ID_REG);
	if(res==MPU_ADDR)//器件ID正确
	{
		MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);	//设置CLKSEL,PLL X轴为参考
		MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);	//加速度与陀螺仪都工作
		MPU_Set_Rate(1000);						//设置采样率为50Hz
 	}else return 1;
	return 0;
}
 
 void calibration(){
	 	float sumx,sumy,sumz;
	for(int a=0;a<1000;a++){
		float temp_x,temp_y,temp_z;
		   short rx,ry,rz;
    u8 buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		rx=((u16)buf[0]<<8)|buf[1];  
		ry=((u16)buf[2]<<8)|buf[3];  
		rz=((u16)buf[4]<<8)|buf[5];
		
		float temp_x=(2000.0*rx)/32768.0;
		float temp_y=(2000.0*ry)/32768.0;
		float temp_z=(2000.0*rz)/32768.0;
	}
		delay_ms(1);
		  sumx+=temp_x;
		sumy+=temp_y;
		sumz+=temp_z;
	}
	 noise_x=sumx/1000;
	noise_y=sumy/1000;
	noise_z=sumz/1000;
 }


