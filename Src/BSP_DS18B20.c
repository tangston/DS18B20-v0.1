#include "BSP_DS18B20.h"
#include "stdlib.h"
#include <stdio.h>
#include "main.h"
#include "gpio.h"
extern u8 Display_Data[8];	

/*
   51移植到STM32中时，每次操作GPIO时，都需要进行管脚输入输出模式的切换
   DS18B20在板级的情况于 main.h的Private define处
*/

/*********************************下方是基本操作**********************************************************************/

/*
   参数
  说明：1位1位的读，每次读完放在最高位，读完左移一位，左移7次得到结果


*/
unsigned char DS18B20_Read_Byte(void)
{

	uchar byte, bi;
	uint  j;
	for (j = 8; j > 0; j--)
	{
		bi =DS18B20_Read_Bit();
		/*将byte左移一位，然后与上右移7位后的bi，注意移动之后移掉那位补0。*/
		byte = (byte >> 1) | (bi << 7);
		delay_us(48); //读取完之后等待48us再接着读取下一个数
	
	}
	return byte;
}

/*
   参数：输入所需要写的字节，延迟函数首地址（名字）
   说明：1位1位的写，每次都是从低位写起


*/
void DS18B20_Write_Byte(unsigned char byte)
{
	uint j;

	for (j = 0; j < 8; j++)
	{
		DS18B20_Write_Bit(byte & 0x01);
		byte >>= 1;
	}
}
/*
	读时序：读时序总时长至少60us，两个读时序之间至少隔1us的总线释放作为恢复时间。读时序以主机总线拉低1us为读准备信号
			主机在读准备信号后15us内（数据有效时间仅15us）需要释放总线并采样
*/
unsigned char DS18B20_Read_Bit()
{
	uchar bi;
  GPIO_OUT(DS18B20_Pin,DS18B20_GPIO_Port);
  HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_RESET);//等效DS18B20 = 0;将总线拉低1us
	delay_us(1);
	HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_SET);//释放总线
  GPIO_IN(DS18B20_Pin,DS18B20_GPIO_Port);
	delay_us(6);//延时6us等待数据稳定
	bi =HAL_GPIO_ReadPin(DS18B20_GPIO_Port,DS18B20_Pin);	 //读取数据，从最低位开始读取
	delay_us(48);		//读取完之后等待48us再接着读取下一个数
	return bi;
}
/*
	写时序：写时序总时长至少60us，两个写时序之间至少隔1us的总线释放作为恢复时间。写时序准备信号为——主机拉低电平至少1us（和读时序一样）
			从机大约在准备信号后15us到60us进行采样。因此，写0、1时，至少主机持续逻辑信号60us以上
*/
void DS18B20_Write_Bit(unsigned char dat)
{
	GPIO_OUT(DS18B20_Pin,DS18B20_GPIO_Port);
  HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_RESET);
	delay_us(1);
  
	HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,dat & 0x01);//写入一个数据，从最低位开始
	delay_us(68);		//延时68us，持续时间最少60us
	HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_SET); //然后释放总线，至少1us给总线恢复时间才能接着写入第二个数值
  GPIO_IN(DS18B20_Pin,DS18B20_GPIO_Port);
  delay_us(1);
	return;
}

/*
    输入参数：延迟函数的首地址（C语言里是这个函数的函数名即可）
    返回值：0为总线上存在器件响应，1为总线上无器件响应
    若有响应，则所有器件在执行完本函数后可以开始使用
*/
unsigned char DS18B20_Reset(void)
{
  u16 i;
	GPIO_OUT(DS18B20_Pin,DS18B20_GPIO_Port);
  HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_RESET); //将总线拉低480us~960us
	delay_us(640);//延时642us
	HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_SET); //然后拉高总线，如果DS18B20做出反应会将在15us~60us后总线拉低
	i = 0;
  GPIO_IN(DS18B20_Pin,DS18B20_GPIO_Port);
	while (HAL_GPIO_ReadPin(DS18B20_GPIO_Port,DS18B20_Pin)) //等待DS18B20拉低总线
	{
		delay_1ms(1);
		i++;
		if (i > 5) //等待>5MS，这里是假设器件在响应别的事情，超过人为界定的限度则说明器件死机
		{
			return 0; //初始化失败
		}
	}
	return 1; //初始化成功
}

/***************************下方为ROM FUNCTION 和 MEMERY FUNCTION指令集操作****************************************************************************/


/*
    note:此函数是从EEPROM中读取Config寄存器，返回精度参数config,若数据校验码不对，则重读3次
    输入参数：@ROM_Code *Code需要配置的器件ROM
    返回值：@unsigned char config：  Temperature_9bit、Temperature_10bit、Temperature_11bit、Temperature_12bit
          config的参数表可以参考DS18B20.h，搜索关键字 “Config Temperature”
          
*/
unsigned char DS18B20_Read_Config_EEPROM(ROM_Code *Code){
    Scratchpad scratchpad1;
    DS18B20_Scratchpad	scratchpad=&scratchpad1;
    DS18B20_Reset();
    DS18B20_Write_Byte(Skip_ROM_Command);//选定所有寄存器  
    DS18B20_Write_Byte(Recall_EEROM_Command);//将EPPROM的值回掉到暂存器中
    DS18B20_Reset();
    /*首先发送指定的器件Code*/
    DS18B20_Write_Byte(Match_ROM_Command);
	if(Code!=NULL){
       
	    /*匹配器件编号*/
		DS18B20_Write_Byte(Code->Family_Code);
		DS18B20_Write_Byte(Code->Serial_Number1);
		DS18B20_Write_Byte(Code->Serial_Number2);
		DS18B20_Write_Byte(Code->Serial_Number3);
		DS18B20_Write_Byte(Code->Serial_Number4);
		DS18B20_Write_Byte(Code->Serial_Number5);
		DS18B20_Write_Byte(Code->Serial_Number6);
		DS18B20_Write_Byte(Code->CRC_Code);
	}
	else /*线上无该器件*/return 0;

	 /*接下来仅有匹配的ROM_CODE的单个元器件响应*/
        DS18B20_Write_Byte(Read_Scratchpad_Command);//发送读暂存器指令
		/*接下来要读取暂存器的九个字节*/
		scratchpad->LSB=DS18B20_Read_Byte();
		scratchpad->MSB=DS18B20_Read_Byte();
        scratchpad->TH=DS18B20_Read_Byte();
        scratchpad->TL=DS18B20_Read_Byte();
        scratchpad->Config=DS18B20_Read_Byte();
        scratchpad->Reserved1=DS18B20_Read_Byte();
        scratchpad->Reserved2=DS18B20_Read_Byte();
        scratchpad->Reserved3=DS18B20_Read_Byte();
        scratchpad->CRC_Code=DS18B20_Read_Byte();
	
	return scratchpad->Config;
}


/*
   
    返回值：@unsigned char config：  Temperature_9bit、Temperature_10bit、Temperature_11bit、Temperature_12bit
          config的参数表可以参考DS18B20.h，搜索关键字 “Config Temperature”
          
*/
unsigned short DS18B20_Read_Temperature(ROM_Code *Code,u16 *TempCMP){
    Scratchpad scratchpad1;
    DS18B20_Scratchpad	scratchpad=&scratchpad1;
    DS18B20_Reset();
    /*首先发送指定的器件Code*/
    DS18B20_Write_Byte(Match_ROM_Command);
	if(Code!=NULL){
       
	    /*匹配器件编号*/
		DS18B20_Write_Byte(Code->Family_Code);
		DS18B20_Write_Byte(Code->Serial_Number1);
		DS18B20_Write_Byte(Code->Serial_Number2);
		DS18B20_Write_Byte(Code->Serial_Number3);
		DS18B20_Write_Byte(Code->Serial_Number4);
		DS18B20_Write_Byte(Code->Serial_Number5);
		DS18B20_Write_Byte(Code->Serial_Number6);
		DS18B20_Write_Byte(Code->CRC_Code);
	}
	else /*线上无该器件*/return 0;

	 /*接下来仅有匹配的ROM_CODE的单个元器件响应*/
	
        DS18B20_Write_Byte(Read_Scratchpad_Command);//发送读暂存器指令
		/*接下来要读取暂存器的九个字节*/
        scratchpad->LSB=DS18B20_Read_Byte();
        scratchpad->MSB=DS18B20_Read_Byte();
        scratchpad->TH=DS18B20_Read_Byte();
        scratchpad->TL=DS18B20_Read_Byte();
        scratchpad->Config=DS18B20_Read_Byte();
        scratchpad->Reserved1=DS18B20_Read_Byte();
        scratchpad->Reserved2=DS18B20_Read_Byte();
        scratchpad->Reserved3=DS18B20_Read_Byte();
        scratchpad->CRC_Code=DS18B20_Read_Byte();
    /*CRC check*/
  
		*TempCMP=scratchpad->LSB;
		*TempCMP |= scratchpad->MSB<<8;
	return scratchpad->Config;
}

/*
    这个一般用在初始化的情况下，统一配置所有DS18B20的Config寄存器，调整输出精度
    note:其中TH/TL这两个字节默认写0，重置脉冲标志着写入结束。DS18B20将此配置固话到EEPROM中需要单独进行
    参数：@unsigned char config：  Temperature_9bit、Temperature_10bit、Temperature_11bit、Temperature_12bit
          @ 是延时函数
          config的参数表可以参考DS18B20.h，搜索关键字 “Config Temperature”
*/
void DS18B20_Config_Accuracy(unsigned char config ){
        DS18B20_Reset();
        DS18B20_Write_Byte(Skip_ROM_Command);//选定所有 
        DS18B20_Write_Byte(Write_Scratchpad_Command);
        DS18B20_Write_Byte(0x00);//TH写入
        DS18B20_Write_Byte(0x00);//TL写入
        DS18B20_Write_Byte(config);//温度精度写入
      /*若不重置器件，则器件重复处于让主机写的状态*/
        DS18B20_Reset();
        DS18B20_Write_Byte(Skip_ROM_Command);//选定所有  
        DS18B20_Write_Byte(Copy_Scratchpad_Command);//将写入的值固化到EEPROM中
}

