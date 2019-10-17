#ifndef __BSP_DS18B20_H
#define __BSP_DS18B20_H


typedef unsigned short u16; //���������ͽ�����������
typedef unsigned char u8;
typedef unsigned int uint; //���������ͽ�����������
typedef unsigned char uchar;
extern void delay_us(u16 time);
extern void delay_1ms(u16 time);
/*��Ҫ�Ľṹ��*/
/*
        �շ����Ǵӵ�λ��ʼ���䣬���ǽ綨���ֶ��ڸ�λ��С�ֶ��ڵ�λ    
        ����64λ��laser ROM����,����MSB�ڸ�λ��LSB�ڵ�λ,�޹ز��ֳ�ʼ��Ϊ0x00
        �����������ݽṹ48λƴ������ CRC_Code + serial_number 8��7...2��1 + Family_Code
*/
typedef struct DS18B20_ROM_Code 
{
     /*serial_numberһ��48λ��6���ֽ�*/
    unsigned char Family_Code;//8-bit
    unsigned char Serial_Number1;//8-bit
    unsigned char Serial_Number2;//8-bit
    unsigned char Serial_Number3;//8-bit
    unsigned char Serial_Number4;//8-bit
    unsigned char Serial_Number5;//8-bit
    unsigned char Serial_Number6;//8-bit
    unsigned char CRC_Code;//8-bit

}ROM_Code,*DS18B20_ROM_Code;



/*
	SCRATCHPAD,�ݴ��������ݽṹ���ֽڷֲ����£��ɵ�λ����λ�Ų�
	LSB�Ͱ�λ�¶�+MSB�߰�λ�¶�+TH�û��澯�¶���������
	+TL�û��澯�¶���������+�¶Ⱦ������ð�λ+�ڲ������ֽ�1��+�ڲ������ֽ�2��+�ڲ������ֽ�3��+CRC
*/
typedef struct DS18B20_Scratchpad
{
    unsigned char LSB;//8-bit
    unsigned char MSB;//8-bit
    unsigned char TH;//8-bit
    unsigned char TL;//8-bit
    unsigned char Config;//8-bit
    unsigned char Reserved1;//8-bit
    unsigned char Reserved2;//8-bit
    unsigned char Reserved3;//8-bit
    unsigned char CRC_Code;//8-bit
}Scratchpad,* DS18B20_Scratchpad;

/*note�����ñ�Ԫ����������ʱ��Ĭ�������GPIO�Ѿ�����ʼ�����������*/


/*ROM Function Command all consist of 1 byte as 8 bits*/
#define Read_ROM_Command  0x33
#define Match_ROM_Command  0x55
#define Skip_ROM_Command  0xCC
#define Search_ROM_Command  0xF0
#define Alarm_ROM_Command  0xEC

/*Memory Command Functions*/
#define Write_Scratchpad_Command  0x4E          //д�ݴ���ָ��
#define Read_Scratchpad_Command  0xBE           //���ݴ���ָ��
#define Copy_Scratchpad_Command  0x48           //���ݴ���ָ������EEROM��
#define Convert_Temperature_Command  0x44       //�¶�ת��ָ��ӻ�Ӧ��0����æ��Ӧ��1�����¶�ת�����
#define Recall_EEROM_Command  0xB8              //�ؼ���EEROMֵ���ݴ�����
#define Read_Power_Supply_Command  0xB4         //�ӻ�Ӧ��0�Ǽ�����Դģʽ��Ӧ��1����ӵ�Դģʽ

/*
    Config Temperature
    ������Ϊ12λ�����¶ȵ�LSB�ֽڵ���Сλ����һλ����δ����ģ�����ֱ����ʧ��
    ����Ϊ11λ�����¶ȵ�LSB�ֽڵĵ�һ����λ��δ����ģ�����ֱ����ʧ��
    ���Ȳ�����   Temperature_9bit �򾫶�Ϊ9λ,0.5�棻     ���ת��ʱ��93.75ms
                Temperature_10bit �򾫶�λ10λ,0.25�棻  ���ת��ʱ��187.5ms
                Temperature_11bit �򾫶�λ11λ,0.125�棻 ���ת��ʱ��375ms
                Temperature_12bit ����Ϊ12λ,0.0625��;   ���ת��ʱ��750ms
    Config�Ĵ����ĵ�5λʼ��Ϊ1��Ҳ����1f
*/
#define Temperature_9bit   (unsigned char)(0x1f)
#define Temperature_10bit  (unsigned char)(0x20+0x1f)
#define Temperature_11bit  (unsigned char)(0x40+0x1f)
#define Temperature_12bit  (unsigned char)(0x60+0x1f)

/*����ָ��*/
unsigned char DS18B20_Read_Bit(void);
void DS18B20_Write_Bit(unsigned char bt);
unsigned char DS18B20_Read_Byte(void);
void DS18B20_Write_Byte(unsigned char byte );
unsigned char DS18B20_Reset(void);


/*����ָ��*/
void DS18B20_Convert_Temperature(void);
unsigned char DS18B20_Busy_Converting(void);
int DS18B20_Read_Temperature_All(void);
void DS18B20_Config_Accuracy(unsigned char config );
unsigned char DS18B20_Read_Config(ROM_Code *Code );
unsigned char DS18B20_Read_Config_EEPROM(ROM_Code *Code );
void DS18B20_Read_ROMCODE(DS18B20_ROM_Code rom_code);
unsigned short DS18B20_Read_Temperature(ROM_Code *Code,u16 *TempCMP);
/*���ߺ���*/
unsigned char CRC_Check(unsigned char *p,char counter);
void Convert_Decode_Completement(unsigned int *Completement);
void Translate_Temperature(unsigned char Temperature_Config,int *integer_Part,float *decimal_part,unsigned int Temperature_Completement);
unsigned char DS18B20_ROM_Search(ROM_Code *Code);


#endif /*__BSP_DS18B20_H*/

