#include "BSP_DS18B20.h"


// definitions
#define FALSE 0
#define TRUE  1


/***************************************************** 
    描    述：  CRC8校验生成多项式 x^8+x^5+x^4+1 
    
******************************************************/  
unsigned char CRC_Table[256]={ 
0,94,188,226,97,63,221,131,194,156,126,32,163,253,31,65,
157,195,33,127,252,162,64,30,95,1,227,189,62,96,130,220,
35,125,159,193,66,28,254,160,225,191,93,3,128,222,60,98,
190,224,2,92,223,129,99,61,124,34,192,158,29,67,161,255,
70,24,250,164,39,121,155,197,132,218,56,102,229,187,89,7,
219,133,103,57,186,228,6,88,25,71,165,251,120,38,196,154,
101,59,217,135,4,90,184,230,167,249,27,69,198,152,122,36,
248,166,68,26,153,199,37,123,58,100,134,216,91,5,231,185,
140,210,48,110,237,179,81,15,78,16,242,172,47,113,147,205,
17,79,173,243,112,46,204,146,211,141,111,49,178,236,14,80,
175,241,19,77,206,144,114,44,109,51,209,143,12,82,176,238,
50,108,142,208,83,13,239,177,240,174,76,18,145,207,45,115,
202,148,118,40,171,245,23,73,8,86,180,234,105,55,213,139,
87,9,235,181,54,104,138,212,149,203,41,119,244,170,72,22,
233,183,85,11,136,214,52,106,43,117,151,201,74,20,246,168,
116,42,200,150,21,75,169,247,182,232,10,84,215,137,107,53};


//几个延时函数，供一线低级操作时调用
//如果改用不同的MPU，如12T，则必须修改这几个函数，确保时间符合协议要求
void Delay480us(void);  //@12.000MHz
void Delay410us(void);  //@12.000MHz
void Delay3_88us(unsigned char i);

//一线低级操作函数
char  OWReset(void);       //复位
void OWWriteBit(char  bt_value);  //写一位
char  OWReadBit(void);     //读一位
void OWWriteByte(unsigned char byte_value); //写一个字节
//////

//搜索函数
char  OWSearch(void);  //算法核心函数，完成一次ROM搜索过程
char  OWFirst(void);   //调用OWSearch完成第一次搜索
char  OWNext(void);   //调用OWSearch完成下一次搜索

//全局搜索变量
unsigned char ROM_NO[8];  //数组，存放本次搜索到的ROM码（8个字节）
char LastDiscrepancy;  //每轮搜索后指向最后一个走0的差异位
char LastFamilyDiscrepancy; //指向家族码（前8位）中最后一个走0的差异位
char LastDeviceFlag;   //搜到最后一个ROM后，程序通过判别将该变量置1，下轮搜索时即会结束退出
unsigned char crc8;    //CRC校验变量
 

//--------------------------------------------------------------------------
//    在单总线上搜索第一个器件
// 返回TRUE: 找到, 存入ROM_NO缓冲；FALSE:无设备
// 先将初始化3个变量，然后调用OWSearch算法进行搜索
//--------------------------------------------------------------------------
char  OWFirst(void)
{
   LastDiscrepancy = 0;
   LastDeviceFlag = FALSE;
   LastFamilyDiscrepancy = 0;
   return OWSearch();
}

//--------------------------------------------------------------------------
//    在单总线上搜索下一个器件
// 返回TRUE: 找到, 存入ROM_NO缓冲；FALSE:无设备，结束搜索
// 在前一轮搜索的基础上（3个变量均在前一轮搜索中有明确的值），再执行一轮搜索
//--------------------------------------------------------------------------
char  OWNext(void)
{
   return OWSearch();
}


//--------------------------------------------------------------------------
//     单总线搜索算法，利用了一些状态变量，算法的核心程序，代码也较长
//     返回TRUEs: 找到, 存入ROM_NO缓冲；FALSE:无设备，结束搜索
//--------------------------------------------------------------------------
char  OWSearch(void)
{
   char id_bt_number;    //指示当前搜索ROM位（取值范围为1-64）
     //下面三个状态变量含义：
  //last_zero：  指针，记录一次搜索（ROM1-64位）最后一位往0走的差异点编号
  //search_direction：搜索某一位时选择的搜索方向（0或1），也是“一写”的bt位值
  //rom_byte_number： ROM字节序号，作为ROM_no[]数组的下标，取值为1—8
   char last_zero, rom_byte_number, search_result;

   char  id_bt, cmp_id_bt,search_direction; //二读（正码、反码）、及一写（决定二叉搜索方向）
   unsigned char rom_byte_mask ; //ROM字节掩码，

   // 初始化本次搜索变量
   id_bt_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = 0;
   crc8 = 0;

// ------------------------------------------------------------------
//1。是否搜索完成（已到最后一个设备）？
//-------------------------------------------------------------------
   if (!LastDeviceFlag)  // LastDeviceFlag由上轮搜索确定是否为最后器件，当然首次进入前必须置False
   {
      if (OWReset())    //复位总线
      {
        
        //如果无应答，返回F，退出本轮搜索程序
         LastDiscrepancy = 0;  //复位几个搜索变量
         LastDeviceFlag = FALSE;
         LastFamilyDiscrepancy = 0;
         return FALSE;    
      }

      OWWriteByte(0xF0);   //发送ROM搜索命令F0H
      Delay3_88us(60);
//=====================================================================
// 开始循环处理1-64位ROM，每位必须进行“二读”后进行判断，确定搜索路径
// 然后按选定的路径进行“一写”，直至完成全部位的搜索，这样一次循环
// 可以完成一轮搜索，找到其中一个ROM码。
//=====================================================================
      do        //逐位读写搜索，1-64位循环
      {
         
         id_bt = OWReadBit();   //二读：先读正码、再读反码
         cmp_id_bt = OWReadBit();

         if (id_bt  && cmp_id_bt)  //二读11，则无器件退出程序   
            break;
         
         
         else       //二读不为11，则需分二种情况
         {
         
            // 第一种情况：01或10，直接可明确搜索方向
            if (id_bt != cmp_id_bt)
               search_direction = id_bt;  // 记下搜索方向search_direction的值待“一写”
            else
            {
               // 否则就是第二种情况：遇到了差异码点，需分三种可能分析：
               // 1。当前位未到达上轮搜索的“最末走0差异码点”（由LastDiscrepancy存储）
               //说明当前经历的是一个老的差异码点，判别特征为当前位在（小于）LastDiscrepancy前
               //不管上次走的是0还是1,只需按上次走的路即可，该值需从ROM_NO中的当前位获取
               if (id_bt_number < LastDiscrepancy)
                  search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               else
               // 2。当前位正好为上轮标记的最末的差异码点，这个差异码点也就是上次走0的点
               //    那么这次就需要走1
               // 3。除去上二种可能，那就是第3种可能： 这是一个新的差异码点，
               //    id_bt_number>LastDiscrepancy
               //。。然而下一条语句巧妙地将上二种可能合在一起处理，看不懂我也没办法了
                  search_direction = (id_bt_number == LastDiscrepancy);
   
        // 确定了差异码点的路径方向还没完事，还需要更新一个指针：last_zero
        // 这个指针每搜索完一位后（注意是一bt不是一轮）总是指向新的差异码点
        // 凡遇到新的差异码点，我们按算法都是先走0，所以凡遇走0的差异码点必须更新此指针
               if (search_direction == 0)
               {
                  last_zero = id_bt_number;

                  // 下面二条是程序的高级功能了：64位ROM中的前8位是器件的家族代码，
                  // 用LastFamilyDiscrepancy这个指针来记录前8位ROM中的最末一个差异码点
                  // 可用于在多类型器件的单线网络中对家族分组进行操作
                  if (last_zero < 9)
                     LastFamilyDiscrepancy = last_zero;
               }
            }
            // 确定了要搜索的方向search_direction，该值即ROM中当前位的值，需要写入ROM
            // 然而64位ROM需分8个字节存入ROM_NO[]，程序使用了一个掩码字节rom_byte_mask
            // 以最低位为例：该字节值为00000001，如记录1则二字节或，写0则与反掩码
            if (search_direction == 1)
               ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
               ROM_NO[rom_byte_number] &= ~rom_byte_mask;

            // 关键的一步操作终于到来了：一写
            OWWriteBit(search_direction);

            // 一个位的操作终于完成，但还需做些工作，以准备下一位的操作：
            // 包括：位变量id_bt_number指向下一位；字节掩码左移一位
            id_bt_number++;
            rom_byte_mask <<= 1;

            // 如果够8位一字节了，则对该字节计算CRC处理、更新字节号变量、重设掩码
            if (rom_byte_mask == 0)
            {
                //docrc8(ROM_NO[rom_byte_number]);  // CRC计算原理参考其他文章
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // ROM bytes编号为 0-7
   //流程图中描述从1到64的位循环，本代码中是利用rom_byte_number<8来判断的
   //至此，终于完成8个字节共64位的循环处理

//=================================================================================
 
      // 一轮搜索成功，找到的一个ROM码也校验OK，则还要处理二个变量
      if (!((id_bt_number < 65) || (crc8 != 0)))
      {
         // 一轮搜索结束后，变量last_zero指向了本轮中最后一个走0的混码位
         // 然后再把此变量保存在LastDiscrepancy中，用于下一轮的判断
         // 当然，last_zero在下轮初始为0，搜索是该变量是不断变动的
        LastDiscrepancy = last_zero;

         // 如果这个指针为0，说明全部搜索结束，再也没有新ROM号器件了
         if (LastDiscrepancy == 0)
            LastDeviceFlag = TRUE;  //设置结束标志
        
         search_result = TRUE;  //返回搜索成功
      }
   }

// ------------------------------------------------------------------
//搜索完成，如果搜索不成功,包括搜索到了,但CRC错误，复位状态变量到首次搜索的状态。
//-------------------------------------------------------------------
   if (!search_result || !ROM_NO[0])
   {
      LastDiscrepancy = 0;
      LastDeviceFlag = FALSE;
      LastFamilyDiscrepancy = 0;
      search_result = FALSE;
   }
   return search_result;
} 

/*
    至此，OWSearch函数结束。函数实现的是一轮搜索，如成功，可得到一个ROM码
*/

/*
     1-Wire函数调用所需的延时函数，注意不同MCU下须重调参数
*/

void Delay480us(void)  //@12.000MHz
{
    delay_us(480);
}

void Delay410us(void)  //@12.000MHz
{
    delay_us(410);
}


void Delay3_88us(unsigned char i)  //@12.000MHz  只能3-80us
{
    delay_us(i);
}
  


/*
      1-wire函数：复位、读一位、写一位、写一字节
*/
char  OWReset(void)
{
        return DS18B20_Reset( ); 
}

//-----------------------------------------------------------------------------
void OWWriteBit(char  dat)  //写一位
{
    DS18B20_Write_Bit((unsigned char)dat); 
}

//-----------------------------------------------------------------------------
char  OWReadBit(void)     //读一位
{
        return DS18B20_Read_Bit( ); 
}

//-----------------------------------------------------------------------------
void OWWriteByte(unsigned char dat) //写一字节
{
        DS18B20_Write_Byte(dat);  
          
}

//-----------------------------------------------------------------------------
char OWReadByte(void)  //读一字节，本例中并未用到
{     
        return DS18B20_Read_Byte( ); 
}


  



