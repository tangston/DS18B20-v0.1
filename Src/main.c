/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "bsp_ds18b20.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//DS18B20 number
#define DS18B20_NO 6


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

ROM_Code Codes[DS18B20_NO];
u8 DS18B20_Number=0;/*the actual number of ds18b20*/
extern unsigned char ROM_NO[8];
extern char OWFirst(void);																									   //when first use OWSearch
extern char OWNext(void);	

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart1 , (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*TIM2 was test for 1us delay */
void delay_us(uint16_t delay)
{
  /*start time base*/
  HAL_TIM_Base_Start(&htim2);
  //use the user counter
    __HAL_TIM_SET_COUNTER(&htim2,0);//��ʼ�������ϼ�ʱ
  while(__HAL_TIM_GET_COUNTER(&htim2)<delay);
  HAL_TIM_Base_Stop(&htim2);
}

/* TIM3 delay for 1ms and maxium is 6553ms*/
void delay_1ms(uint16_t delay)
{
   /*start time base*/
   HAL_TIM_Base_Start(&htim3);
 //use the user counter
    __HAL_TIM_SET_COUNTER(&htim3,0);
  while(__HAL_TIM_GET_COUNTER(&htim3)<delay*10);
   HAL_TIM_Base_Stop(&htim3);
}

/*
    
*/
float show_Temp_uart(ROM_Code *ROM_NO){
  
  unsigned char T_Config=0,i=0,IsNegetive=0;
	u16 Tempcomplement;
	float Temperature;
  
	printf(" ");
 
	T_Config= DS18B20_Read_Temperature(ROM_NO,&Tempcomplement);//return for the temp_config

	/*int16*/
	if((short)Tempcomplement<0){
		Tempcomplement=~(Tempcomplement-1);
		IsNegetive=1;
	}
    /*different T_config is different accuracy*/
	 switch (T_Config)
    {
        case Temperature_9bit:Tempcomplement &= 0xfff8; printf("Config is 9... "); //reset the last 3 bit to 0
        break;
        case Temperature_10bit:Tempcomplement &= 0xfffb;printf("Config is 10... "); //reset the last 2 bit to 0
        break;
        case Temperature_11bit:Tempcomplement &= 0xfffe;printf("Config is 11... "); //reset the last 1 bit to 0            
        break;
        case Temperature_12bit:printf("Config is 12 ..."); /*do nothing*/
        break;
    }
    Temperature=Tempcomplement*0.0625;
    printf("T is %.4f... \n",Temperature);
    return Temperature;
}

/*
	copy ROM Code from the global ROM search
*/
void ROM_Copy(unsigned char *rom_code, unsigned char *ROM_NO)
{

	int i = 0;
	for (; i < 8; ++i)
	{
		rom_code[i] = ROM_NO[i];
	}
	return;
}
void print_ROM_Code(unsigned char *ROM_NO){
   printf("0x%02x %02x %02x %02x %02x %02x %02x %02x \n",ROM_NO[0],ROM_NO[1],ROM_NO[2],ROM_NO[3],ROM_NO[4],ROM_NO[5],ROM_NO[6],ROM_NO[7]);
  
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /*  */
  char result;
   u8 i=0;
  unsigned char *pcode=NULL;
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  i=0;
  /**/
  printf("Searching DS18B20......\n");
  result= OWFirst();
  printf("the NO.%d ROM :0x%02x %02x %02x %02x %02x %02x %02x %02x \n",i,ROM_NO[0],ROM_NO[1],ROM_NO[2],ROM_NO[3],ROM_NO[4],ROM_NO[5],ROM_NO[6],ROM_NO[7]);
  pcode=&(Codes[i].Family_Code);
  ROM_Copy(pcode,(unsigned char *)&ROM_NO);
  ++i;
 while (result)
	{
		result = OWNext();
    if(result){    
      printf("the NO.%d ROM :0x%02x %02x %02x %02x %02x %02x %02x %02x \n",i,ROM_NO[0],ROM_NO[1],ROM_NO[2],ROM_NO[3],ROM_NO[4],ROM_NO[5],ROM_NO[6],ROM_NO[7]);
      pcode=&(Codes[i].Family_Code);
      ROM_Copy(pcode,(unsigned char *)&ROM_NO);
      ++i;
    }
	}
  printf("Search ROM done,%d devices detected\n",i);
  DS18B20_Number=i;
  /* USER CODE END 2 */

  /* Infinite loop */
  printf("Stored ROM Codes %d:\n",DS18B20_Number);
  for(i=0;i<DS18B20_Number;++i){
     printf("Stored the NO.%d ROM :",i+1);
     pcode=&(Codes[i].Family_Code);
     print_ROM_Code(pcode);
  }
  
  
  /* USER CODE BEGIN WHILE */
   printf("Now reading temperature,ordering by the rom_Seach up above\n\n");
  while (1)
  {
    
     delay_1ms(1000);//
     DS18B20_Reset();
     DS18B20_Write_Byte(Skip_ROM_Command);
     DS18B20_Write_Byte(Convert_Temperature_Command);
     delay_1ms(500);
    for(i=0;i<DS18B20_Number;++i)
     show_Temp_uart(&Codes[i]); 
    printf("\n\n");
  
    /**/
    
    
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
