#include "BSP_DS18B20.h"
#include "stdlib.h"
#include <stdio.h>
#include "main.h"
#include "gpio.h"
extern u8 Display_Data[8];	

/*
   51��ֲ��STM32��ʱ��ÿ�β���GPIOʱ������Ҫ���йܽ��������ģʽ���л�
   DS18B20�ڰ弶������� main.h��Private define��
*/

/*********************************�·��ǻ�������**********************************************************************/

/*
   ����
  ˵����1λ1λ�Ķ���ÿ�ζ���������λ����������һλ������7�εõ����


*/
unsigned char DS18B20_Read_Byte(void)
{

	uchar byte, bi;
	uint  j;
	for (j = 8; j > 0; j--)
	{
		bi =DS18B20_Read_Bit();
		/*��byte����һλ��Ȼ����������7λ���bi��ע���ƶ�֮���Ƶ���λ��0��*/
		byte = (byte >> 1) | (bi << 7);
		delay_us(48); //��ȡ��֮��ȴ�48us�ٽ��Ŷ�ȡ��һ����
	
	}
	return byte;
}

/*
   ��������������Ҫд���ֽڣ��ӳٺ����׵�ַ�����֣�
   ˵����1λ1λ��д��ÿ�ζ��Ǵӵ�λд��


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
	��ʱ�򣺶�ʱ����ʱ������60us��������ʱ��֮�����ٸ�1us�������ͷ���Ϊ�ָ�ʱ�䡣��ʱ����������������1usΪ��׼���ź�
			�����ڶ�׼���źź�15us�ڣ�������Чʱ���15us����Ҫ�ͷ����߲�����
*/
unsigned char DS18B20_Read_Bit()
{
	uchar bi;
  GPIO_OUT(DS18B20_Pin,DS18B20_GPIO_Port);
  HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_RESET);//��ЧDS18B20 = 0;����������1us
	delay_us(1);
	HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_SET);//�ͷ�����
  GPIO_IN(DS18B20_Pin,DS18B20_GPIO_Port);
	delay_us(6);//��ʱ6us�ȴ������ȶ�
	bi =HAL_GPIO_ReadPin(DS18B20_GPIO_Port,DS18B20_Pin);	 //��ȡ���ݣ������λ��ʼ��ȡ
	delay_us(48);		//��ȡ��֮��ȴ�48us�ٽ��Ŷ�ȡ��һ����
	return bi;
}
/*
	дʱ��дʱ����ʱ������60us������дʱ��֮�����ٸ�1us�������ͷ���Ϊ�ָ�ʱ�䡣дʱ��׼���ź�Ϊ�����������͵�ƽ����1us���Ͷ�ʱ��һ����
			�ӻ���Լ��׼���źź�15us��60us���в�������ˣ�д0��1ʱ���������������߼��ź�60us����
*/
void DS18B20_Write_Bit(unsigned char dat)
{
	GPIO_OUT(DS18B20_Pin,DS18B20_GPIO_Port);
  HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_RESET);
	delay_us(1);
  
	HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,dat & 0x01);//д��һ�����ݣ������λ��ʼ
	delay_us(68);		//��ʱ68us������ʱ������60us
	HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_SET); //Ȼ���ͷ����ߣ�����1us�����߻ָ�ʱ����ܽ���д��ڶ�����ֵ
  GPIO_IN(DS18B20_Pin,DS18B20_GPIO_Port);
  delay_us(1);
	return;
}

/*
    ����������ӳٺ������׵�ַ��C����������������ĺ��������ɣ�
    ����ֵ��0Ϊ�����ϴ���������Ӧ��1Ϊ��������������Ӧ
    ������Ӧ��������������ִ���걾��������Կ�ʼʹ��
*/
unsigned char DS18B20_Reset(void)
{
  u16 i;
	GPIO_OUT(DS18B20_Pin,DS18B20_GPIO_Port);
  HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_RESET); //����������480us~960us
	delay_us(640);//��ʱ642us
	HAL_GPIO_WritePin(DS18B20_GPIO_Port,DS18B20_Pin,GPIO_PIN_SET); //Ȼ���������ߣ����DS18B20������Ӧ�Ὣ��15us~60us����������
	i = 0;
  GPIO_IN(DS18B20_Pin,DS18B20_GPIO_Port);
	while (HAL_GPIO_ReadPin(DS18B20_GPIO_Port,DS18B20_Pin)) //�ȴ�DS18B20��������
	{
		delay_1ms(1);
		i++;
		if (i > 5) //�ȴ�>5MS�������Ǽ�����������Ӧ������飬������Ϊ�綨���޶���˵����������
		{
			return 0; //��ʼ��ʧ��
		}
	}
	return 1; //��ʼ���ɹ�
}

/***************************�·�ΪROM FUNCTION �� MEMERY FUNCTIONָ�����****************************************************************************/


/*
    note:�˺����Ǵ�EEPROM�ж�ȡConfig�Ĵ��������ؾ��Ȳ���config,������У���벻�ԣ����ض�3��
    ���������@ROM_Code *Code��Ҫ���õ�����ROM
    ����ֵ��@unsigned char config��  Temperature_9bit��Temperature_10bit��Temperature_11bit��Temperature_12bit
          config�Ĳ�������Բο�DS18B20.h�������ؼ��� ��Config Temperature��
          
*/
unsigned char DS18B20_Read_Config_EEPROM(ROM_Code *Code){
    Scratchpad scratchpad1;
    DS18B20_Scratchpad	scratchpad=&scratchpad1;
    DS18B20_Reset();
    DS18B20_Write_Byte(Skip_ROM_Command);//ѡ�����мĴ���  
    DS18B20_Write_Byte(Recall_EEROM_Command);//��EPPROM��ֵ�ص����ݴ�����
    DS18B20_Reset();
    /*���ȷ���ָ��������Code*/
    DS18B20_Write_Byte(Match_ROM_Command);
	if(Code!=NULL){
       
	    /*ƥ���������*/
		DS18B20_Write_Byte(Code->Family_Code);
		DS18B20_Write_Byte(Code->Serial_Number1);
		DS18B20_Write_Byte(Code->Serial_Number2);
		DS18B20_Write_Byte(Code->Serial_Number3);
		DS18B20_Write_Byte(Code->Serial_Number4);
		DS18B20_Write_Byte(Code->Serial_Number5);
		DS18B20_Write_Byte(Code->Serial_Number6);
		DS18B20_Write_Byte(Code->CRC_Code);
	}
	else /*�����޸�����*/return 0;

	 /*����������ƥ���ROM_CODE�ĵ���Ԫ������Ӧ*/
        DS18B20_Write_Byte(Read_Scratchpad_Command);//���Ͷ��ݴ���ָ��
		/*������Ҫ��ȡ�ݴ����ľŸ��ֽ�*/
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
   
    ����ֵ��@unsigned char config��  Temperature_9bit��Temperature_10bit��Temperature_11bit��Temperature_12bit
          config�Ĳ�������Բο�DS18B20.h�������ؼ��� ��Config Temperature��
          
*/
unsigned short DS18B20_Read_Temperature(ROM_Code *Code,u16 *TempCMP){
    Scratchpad scratchpad1;
    DS18B20_Scratchpad	scratchpad=&scratchpad1;
    DS18B20_Reset();
    /*���ȷ���ָ��������Code*/
    DS18B20_Write_Byte(Match_ROM_Command);
	if(Code!=NULL){
       
	    /*ƥ���������*/
		DS18B20_Write_Byte(Code->Family_Code);
		DS18B20_Write_Byte(Code->Serial_Number1);
		DS18B20_Write_Byte(Code->Serial_Number2);
		DS18B20_Write_Byte(Code->Serial_Number3);
		DS18B20_Write_Byte(Code->Serial_Number4);
		DS18B20_Write_Byte(Code->Serial_Number5);
		DS18B20_Write_Byte(Code->Serial_Number6);
		DS18B20_Write_Byte(Code->CRC_Code);
	}
	else /*�����޸�����*/return 0;

	 /*����������ƥ���ROM_CODE�ĵ���Ԫ������Ӧ*/
	
        DS18B20_Write_Byte(Read_Scratchpad_Command);//���Ͷ��ݴ���ָ��
		/*������Ҫ��ȡ�ݴ����ľŸ��ֽ�*/
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
    ���һ�����ڳ�ʼ��������£�ͳһ��������DS18B20��Config�Ĵ����������������
    note:����TH/TL�������ֽ�Ĭ��д0�����������־��д�������DS18B20�������ù̻���EEPROM����Ҫ��������
    ������@unsigned char config��  Temperature_9bit��Temperature_10bit��Temperature_11bit��Temperature_12bit
          @ ����ʱ����
          config�Ĳ�������Բο�DS18B20.h�������ؼ��� ��Config Temperature��
*/
void DS18B20_Config_Accuracy(unsigned char config ){
        DS18B20_Reset();
        DS18B20_Write_Byte(Skip_ROM_Command);//ѡ������ 
        DS18B20_Write_Byte(Write_Scratchpad_Command);
        DS18B20_Write_Byte(0x00);//THд��
        DS18B20_Write_Byte(0x00);//TLд��
        DS18B20_Write_Byte(config);//�¶Ⱦ���д��
      /*���������������������ظ�����������д��״̬*/
        DS18B20_Reset();
        DS18B20_Write_Byte(Skip_ROM_Command);//ѡ������  
        DS18B20_Write_Byte(Copy_Scratchpad_Command);//��д���ֵ�̻���EEPROM��
}

