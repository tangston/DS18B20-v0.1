#ifndef __BSP_DS18B20_H
#define __BSP_DS18B20_H


typedef unsigned short u16; //对数据类型进行声明定义
typedef unsigned char u8;
typedef unsigned int uint; //对数据类型进行声明定义
typedef unsigned char uchar;
extern void delay_us(u16 time);
extern void delay_1ms(u16 time);
/*需要的结构体*/
/*
        收发都是从低位开始传输，我们界定大字端在高位，小字端在低位    
        这是64位的laser ROM编码,都是MSB在高位，LSB在低位,无关部分初始化为0x00
        合着整个数据结构48位拼起来是 CRC_Code + serial_number 8、7...2、1 + Family_Code
*/
typedef struct DS18B20_ROM_Code 
{
     /*serial_number一共48位，6个字节*/
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
	SCRATCHPAD,暂存器的数据结构，字节分布如下，由低位到高位排布
	LSB低八位温度+MSB高八位温度+TH用户告警温度上限设置
	+TL用户告警温度下限设置+温度精度配置八位+内部计算字节1号+内部计算字节2号+内部计算字节3号+CRC
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

/*note：调用本元器件操作包时，默认下面的GPIO已经被初始化配置完成了*/


/*ROM Function Command all consist of 1 byte as 8 bits*/
#define Read_ROM_Command  0x33
#define Match_ROM_Command  0x55
#define Skip_ROM_Command  0xCC
#define Search_ROM_Command  0xF0
#define Alarm_ROM_Command  0xEC

/*Memory Command Functions*/
#define Write_Scratchpad_Command  0x4E          //写暂存器指令
#define Read_Scratchpad_Command  0xBE           //读暂存器指令
#define Copy_Scratchpad_Command  0x48           //将暂存器指拷贝到EEROM中
#define Convert_Temperature_Command  0x44       //温度转化指令，从机应答0代表忙，应答1代表温度转化完成
#define Recall_EEROM_Command  0xB8              //重加载EEROM值到暂存器中
#define Read_Power_Supply_Command  0xB4         //从机应答0是寄生电源模式，应答1是外接电源模式

/*
    Config Temperature
    若精度为12位，则温度的LSB字节的最小位（第一位）是未定义的（精度直接损失）
    精度为11位，则温度的LSB字节的第一、二位是未定义的（精度直接损失）
    精度参数：   Temperature_9bit 则精度为9位,0.5℃；     最大转化时间93.75ms
                Temperature_10bit 则精度位10位,0.25℃；  最大转化时间187.5ms
                Temperature_11bit 则精度位11位,0.125℃； 最大转化时间375ms
                Temperature_12bit 精度为12位,0.0625℃;   最大转化时间750ms
    Config寄存器的低5位始终为1，也就是1f
*/
#define Temperature_9bit   (unsigned char)(0x1f)
#define Temperature_10bit  (unsigned char)(0x20+0x1f)
#define Temperature_11bit  (unsigned char)(0x40+0x1f)
#define Temperature_12bit  (unsigned char)(0x60+0x1f)

/*基础指令*/
unsigned char DS18B20_Read_Bit(void);
void DS18B20_Write_Bit(unsigned char bt);
unsigned char DS18B20_Read_Byte(void);
void DS18B20_Write_Byte(unsigned char byte );
unsigned char DS18B20_Reset(void);


/*常规指令*/
void DS18B20_Convert_Temperature(void);
unsigned char DS18B20_Busy_Converting(void);
int DS18B20_Read_Temperature_All(void);
void DS18B20_Config_Accuracy(unsigned char config );
unsigned char DS18B20_Read_Config(ROM_Code *Code );
unsigned char DS18B20_Read_Config_EEPROM(ROM_Code *Code );
void DS18B20_Read_ROMCODE(DS18B20_ROM_Code rom_code);
unsigned short DS18B20_Read_Temperature(ROM_Code *Code,u16 *TempCMP);
/*工具函数*/
unsigned char CRC_Check(unsigned char *p,char counter);
void Convert_Decode_Completement(unsigned int *Completement);
void Translate_Temperature(unsigned char Temperature_Config,int *integer_Part,float *decimal_part,unsigned int Temperature_Completement);
unsigned char DS18B20_ROM_Search(ROM_Code *Code);


#endif /*__BSP_DS18B20_H*/

