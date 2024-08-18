/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "extern.h"
#include <string.h>
#include <stdlib.h>
#include "mylibrary.h"
#include "math.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
DMA_HandleTypeDef hdma_tim1_ch1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char buffer[20];
int a =0;
int ia=0;
int iz=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define MAX_LED 160
#define USE_BRIGHTNESS 1 //밝기조절 필요하면 1, 필요없으면 0


uint8_t LED_Data[MAX_LED][4]; //LED_Data는 개별 LED 색상 저장하는데 사용
uint8_t LED_Mod[MAX_LED][4];  // 밝기설정에 따라 스케일링 된 값

int datasentflag=0;

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
	datasentflag=1;
}

void Set_LED (int LEDnum, int Red, int Green, int Blue)
{
	LED_Data[LEDnum][0] = LEDnum;
	LED_Data[LEDnum][1] = Green;
	LED_Data[LEDnum][2] = Red;
	LED_Data[LEDnum][3] = Blue;
}

#define PI 3.14159265

void Set_Brightness (int brightness)  // 0-45 : 밝기는 0-255 사이이고 TAN함수로 선형적으로 만듦 
{
#if USE_BRIGHTNESS

	if (brightness > 45) brightness = 45;
	for (int i=0; i<MAX_LED; i++)
	{
		LED_Mod[i][0] = LED_Data[i][0];
		for (int j=1; j<4; j++)
		{
			float angle = 90-brightness;  // in degrees
			angle = angle*PI / 180;  // in rad
			LED_Mod[i][j] = (LED_Data[i][j])/(tan(angle));
		}
	}

#endif
}

uint16_t pwmData[(24*MAX_LED)+80];

void SK6812_Send (void)
{
	uint32_t indx=0;
	uint32_t color;


	for (int i= 0; i<MAX_LED; i++)
	{
#if USE_BRIGHTNESS
		color = ((LED_Mod[i][1]<<16) | (LED_Mod[i][2]<<8) | (LED_Mod[i][3]));
#else
		color = ((LED_Data[i][1]<<16) | (LED_Data[i][2]<<8) | (LED_Data[i][3]));
#endif

		for (int i=23; i>=0; i--)
		{
			if (color&(1<<i))
			{
				pwmData[indx] = 67;  // 3/4 of 90
			}

			else pwmData[indx] = 22;  // 1/4 of 90

			indx++;
		}

	}

	for (int i=0; i<80; i++)
	{
		pwmData[indx] = 0;
		indx++;
	}

	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)pwmData, indx);
	while (!datasentflag){};
	datasentflag = 0;
}

void Reset_LED (void)
{
	for (int i=0; i<MAX_LED; i++)
	{
		LED_Data[i][0] = i;
		LED_Data[i][1] = 0;
		LED_Data[i][2] = 0;
		LED_Data[i][3] = 0;
	}
}
  int rz = 10;
  int gz = 15;
  int bz = 30;
  

// 무지개 색상데이터
  
  ////////////////////////////////
 uint16_t effStep = 0;

uint8_t rainbow_effect_left() {
    // Strip ID: 0 - Effect: Rainbow - LEDS: 160
    // Steps: 8 - Delay: 31
    // Colors: 3 (255.0.0, 0.255.0, 0.0.255)
    // Options: rainbowlen=1, toLeft=true, 

  float factor1, factor2;
  uint16_t ind;
  for(uint16_t j=0;j<160;j++) {
    ind = effStep + j * 8;
    switch((int)((ind % 8) / 2.6666666666666665)) {
      case 0: factor1 = 1.0 - ((float)(ind % 8 - 0 * 2.6666666666666665) / 2.6666666666666665);
              factor2 = (float)((int)(ind - 0) % 8) / 2.6666666666666665;
              Set_LED(j, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
			  Set_Brightness(20);
		      SK6812_Send();
              break;
      case 1: factor1 = 1.0 - ((float)(ind % 8 - 1 * 2.6666666666666665) / 2.6666666666666665);
              factor2 = (float)((int)(ind - 2.6666666666666665) % 8) / 2.6666666666666665;
              Set_LED(j, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
			  Set_Brightness(20);
		      SK6812_Send();
              break;
      case 2: factor1 = 1.0 - ((float)(ind % 8 - 2 * 2.6666666666666665) / 2.6666666666666665);
              factor2 = (float)((int)(ind - 5.333333333333333) % 8) / 2.6666666666666665;
              Set_LED(j, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
			  Set_Brightness(20);
		      SK6812_Send();
              break;
    }
  }
  if(effStep >= 8) {effStep=0; return 0x03; }
  else effStep++;
  return 0x01;
}
uint8_t timer_count = 0, buffer_index = 0;

uint8_t string_compare(char array1[], char array2[], uint16_t length)
{
	 uint8_t comVAR=0, i;
	 for(i=0;i<length;i++)
	   	{
	   		  if(array1[i]==array2[i])
	   	  		  comVAR++;
	   	  	  else comVAR=0;
	   	}
	 if (comVAR==length)
		 	return 1;
	 else 	return 0;
}
// 색상 값 및 While 탈출식
void Message_handler()
{
	    if(string_compare(buffer, "ledon", strlen("ledon")))
	{
a=3;
          
		//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_SET);
               // HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_SET);
              //  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_SET);

		HAL_UART_Transmit(&huart2, (uint8_t*)"LED is ON.\n", strlen("LED is ON.\n"), 200);
	}else
	if(string_compare(buffer, "ledoff", strlen("ledoff")))
	{
          a=1;
              // HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
             //  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
            //  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
	//	HAL_UART_Transmit(&huart2, (uint8_t*)"LED is OFF.\n", strlen("LED is OFF.\n"), 500);
                         
	}else
          
                    if(string_compare(buffer, "red", strlen("red")))
	{
			iz=0, a=0;
                rz=100,bz=0,gz=0;
               // HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_RESET);
             //   HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_SET);
           //    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);

		HAL_UART_Transmit(&huart2, (uint8_t*)"RED.\n", strlen("RED.\n"), 500);
	}else
          if(string_compare(buffer, "blue", strlen("blue")))
	{
			iz=0, a=0;		
                rz=0,bz=100,gz=0;
              // HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_SET);
            //   HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
            //  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);
   
		HAL_UART_Transmit(&huart2, (uint8_t*)"BLUE.\n", strlen("BLUE.\n"), 500);
	}else
          if(string_compare(buffer, "gree", strlen("gree")))
	{
			iz=0, a=0;
                rz=0,bz=0,gz=100;
            //  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_RESET);
             //  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
                //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_SET);
		HAL_UART_Transmit(&huart2, (uint8_t*)"GREEN.\n", strlen("GREEN.\n"), 500);
	}else
           if(string_compare(buffer, "ora", strlen("ora")))
	{
			iz=0, a=0;
                rz=100,bz=0,gz=40;
		HAL_UART_Transmit(&huart2, (uint8_t*)"ORANG.\n", strlen("ORANG.\n"), 500);
	}else 
          if(string_compare(buffer, "yel", strlen("yel")))
	{
		iz=0, a=0;
                rz=100,bz=0,gz=100;
		HAL_UART_Transmit(&huart2, (uint8_t*)"YELLOW. \n", strlen("YELLO.\n"), 500);
	}else
          if(string_compare(buffer, "aqua", strlen("aqua")))
	{
			iz=0, a=0;
                rz=0,bz=100,gz=100;
		HAL_UART_Transmit(&huart2, (uint8_t*)"AQUA.\n", strlen("AQUA.\n"), 500);
	}else
           if(string_compare(buffer, "pur", strlen("pur")))
	{
			iz=0, a=0;
                rz=100,bz=100,gz=0;
		HAL_UART_Transmit(&huart2, (uint8_t*)"PURPLE.\n", strlen("PURPLE.\n"), 500);
	}else
          if(string_compare(buffer, "come", strlen("come")))
	{
			iz=0, a=0;
		       a=50;
		HAL_UART_Transmit(&huart2, (uint8_t*)"LED Brightness TEST.\n", strlen("LED Brightness TEST.\n"), 500);
	}else
          if(string_compare(buffer, "rain", strlen("rain")))
	{
			iz=0;
		        a=2;
                HAL_UART_Transmit(&huart2, (uint8_t*)"RAINBOW.\n", strlen("RAINBOW.\n"), 500);
	}
          else
            if(string_compare(buffer, "pin", strlen("pin")))
	{
			iz=0, a=0;
          rz=255,bz=45,gz=51;
                HAL_UART_Transmit(&huart2, (uint8_t*)"PINK.\n", strlen("PINK.\n"), 500);
	}
          else
            if(string_compare(buffer, "whi", strlen("whi")))
	{
			iz=0, a=0;
          rz=150,bz=150,gz=150;
                HAL_UART_Transmit(&huart2, (uint8_t*)"WHITE.\n", strlen("WHITE.\n"), 500);
	}
          else
            if(string_compare(buffer, "cju", strlen("cju")))
	{
          rz=80,bz=15,gz=45;

                HAL_UART_Transmit(&huart2, (uint8_t*)"CJU.\n", strlen("CJU.\n"), 500);
	}
          else
	{
		strcat(buffer, "\n");
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 500);
	}

	memset(buffer, 0x00, sizeof(buffer));
	buffer_index = 0;
	timer_count = 0;
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_DMA_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  static char toggle[4] = {0,};
  static uint32_t tick_Seconds = 0;
  
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  tick_Seconds = sTime.Seconds;
 
// 이 코드를 추가해야 한다. 함수 원형은 이렇게 생겼다.
  // HAL_StatusTypeDef HAL_UART_Receive_IT
  //                  (UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)      
//  HAL_UART_Receive_IT(&huart2, (uint8_t*)uart2_rx_data, 1);  
//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  
 // HAL_Delay(200);
 // HAL_GPIO_WritePin(BuletoothReset_GPIO_Port, BuletoothReset_Pin, GPIO_PIN_SET);
 // HAL_Delay(1000);
 // memset(buffer, 0x00, sizeof(buffer));
  HAL_TIM_Base_Start_IT(&htim3);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
/*
if(sTime.Seconds == 0 && tick_Seconds == 59)
{
	tick_Seconds = 0;
	toggle[0] = 0;
    printf("%04d-%02d-%02d -- %02d:%02d:%02d\r\n",sDate.Year+2000,sDate.Month,sDate.Date,sTime.Hours,sTime.Minutes,sTime.Seconds);
  }
  */
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

// RTC식
if(string_compare(buffer, "timon", strlen("timon")))
{
    if(sTime.Seconds  > tick_Seconds)
    {
      tick_Seconds = sTime.Seconds;   
      toggle[0] ^= 1;
      if(toggle[0]){

        printf("%04d-%02d-%02d -- %02d:%02d:%02d\r\n",sDate.Year+2000,sDate.Month,sDate.Date,sTime.Hours,sTime.Minutes,sTime.Seconds);
      }
      else {
;
        printf("%04d-%02d-%02d -- %02d:%02d:%02d\r\n",sDate.Year+2000,sDate.Month,sDate.Date,sTime.Hours,sTime.Minutes,sTime.Seconds);
HAL_UART_Transmit(&huart2, (uint8_t*)"", strlen(""), 500);
      }
	}
  }


// 밝기 제어식

		  if(string_compare(buffer, "danA", strlen("danA")))
			{
			iz=0;
			Set_Brightness(0);
			SK6812_Send();
			}
		  else  
			if(string_compare(buffer, "danB", strlen("danB")))
			{
			//	for (int i=0; i<5; i++)
		//	{
			iz=5;
			Set_Brightness(5);
			SK6812_Send();
	//		}
			}
		  else  
			if(string_compare(buffer, "danC", strlen("danC")))
			{
	//			for (int i=5; i<10; i++)
	//		{
			iz=10;
			Set_Brightness(10);
			SK6812_Send();
	//		}
			}
		  if(string_compare(buffer, "danD", strlen("danD")))
			{
			iz=15;
			Set_Brightness(15);
			SK6812_Send();
			}
		  else  
			if(string_compare(buffer, "danE", strlen("danE")))
			{
			iz=20;
			Set_Brightness(20);
			SK6812_Send();
			}
		  else  
			if(string_compare(buffer, "danF", strlen("danF")))
			{
			iz=25;
			Set_Brightness(25);
			SK6812_Send();
			}
		  else  
			if(string_compare(buffer, "danG", strlen("danG")))
			{
			iz=30;
			Set_Brightness(30);
			SK6812_Send();
			}
		  else  
			if(string_compare(buffer, "danH", strlen("danH")))
			{
			iz=35;
			Set_Brightness(35);
			SK6812_Send();
			}
		  if(string_compare(buffer, "danI", strlen("danI")))
			{
			iz=40;
			Set_Brightness(40);
			SK6812_Send();
			}
		  else  
			if(string_compare(buffer, "danJ", strlen("danJ")))
			{
			iz=45;
			Set_Brightness(45);
			SK6812_Send();
			}


if(string_compare(buffer, "RA", strlen("RA")))
			{
 				rz=0;
				iz=0, a=0;
			}
else
if(string_compare(buffer, "RB", strlen("RB")))
			{
 				rz=15;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RC", strlen("RC")))
			{
 				rz=30;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RD", strlen("RD")))
			{
 				rz=45;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RE", strlen("RE")))
			{
 				rz=60;
				iz=1, a=0;
			}
else

if(string_compare(buffer, "RF", strlen("RF")))
			{
 				rz=75;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RG", strlen("RG")))
			{
 				rz=90;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RH", strlen("RH")))
			{
 				rz=105;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RI", strlen("RI")))
			{
 				rz=120;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RJ", strlen("RJ")))
			{
 				rz=135;
				iz=1, a=0;
			}
else

if(string_compare(buffer, "RK", strlen("RK")))
			{
 				rz=150;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RL", strlen("RL")))
			{
 				rz=165;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RM", strlen("RM")))
			{
 				rz=180;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RN", strlen("RN")))
			{
 				rz=195;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RO", strlen("RO")))
			{
 				rz=210;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "RP", strlen("RP")))
			{
 				rz=225;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GA", strlen("GA")))
			{
 				gz=0;
				iz=0, a=0;
			}
else
if(string_compare(buffer, "GB", strlen("GB")))
			{
 				gz=15;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GC", strlen("GC")))
			{
 				gz=30;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GD", strlen("GD")))
			{
 				gz=45;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GE", strlen("GE")))
			{
 				gz=60;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GF", strlen("GF")))
			{
 				gz=75;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GG", strlen("GG")))
			{
 				gz=90;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GH", strlen("GH")))
			{
 				gz=105;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GI", strlen("GI")))
			{
 				gz=120;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GJ", strlen("GJ")))
			{
 				gz=135;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GK", strlen("GK")))
			{
 				gz=150;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GL", strlen("GL")))
			{
 				gz=165;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GM", strlen("GM")))
			{
 				gz=180;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GN", strlen("GN")))
			{
 				gz=195;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GO", strlen("GO")))
			{
 				gz=210;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "GP", strlen("GP")))
			{
 				gz=225;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BA", strlen("BA")))
			{
 				bz=0;
				iz=0, a=0;
			}
else
if(string_compare(buffer, "BB", strlen("BB")))
			{
 				bz=15;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BC", strlen("BC")))
			{
 				bz=30;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BD", strlen("BD")))
			{
 				bz=45;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BE", strlen("BE")))
			{
 				bz=60;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BF", strlen("BF")))
			{
 				bz=75;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BG", strlen("BG")))
			{
 				bz=90;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BH", strlen("BH")))
			{
 				bz=105;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BI", strlen("BI")))
			{
 				bz=120;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BJ", strlen("BJ")))
			{
 				bz=135;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BK", strlen("BK")))
			{
 				bz=150;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BL", strlen("BL")))
			{
 				bz=165;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BM", strlen("BM")))
			{
 				bz=180;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BN", strlen("BN")))
			{
 				bz=195;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BO", strlen("BO")))
			{
 				bz=210;
				iz=1, a=0;
			}
else
if(string_compare(buffer, "BP", strlen("BP")))
			{
 				bz=225;
				iz=1, a=0;
			}
// 색상 설정 동시에 ON

                    if(string_compare(buffer, "redon", strlen("redon")))
	{
			iz=0;
                rz=100,bz=0,gz=0;
	Set_Brightness(25);
	SK6812_Send();




	}else
          if(string_compare(buffer, "blueon", strlen("blueon")))
	{
			iz=0;	
                rz=0,bz=100,gz=0;
	Set_Brightness(25);
	SK6812_Send();


   

	}else
          if(string_compare(buffer, "greenon", strlen("greenon")))
	{
			iz=0;
                rz=0,bz=0,gz=100;
	Set_Brightness(25);
	SK6812_Send();



	}else
           if(string_compare(buffer, "oraon", strlen("oraon")))
	{
			iz=0;
                rz=100,bz=0,gz=40;
	Set_Brightness(25);
	SK6812_Send();


	}else 
          if(string_compare(buffer, "yelon", strlen("yelon")))
	{
			iz=0;
                rz=50,bz=0,gz=50;
	Set_Brightness(25);
	SK6812_Send();


	}else
          if(string_compare(buffer, "aquaon", strlen("aquaon")))
	{
			iz=0;
                rz=0,bz=100,gz=100;
	Set_Brightness(25);
	SK6812_Send();


	}else
           if(string_compare(buffer, "puron", strlen("puron")))
	{
			iz=0;
                rz=100,bz=100,gz=0;
	Set_Brightness(25);
	SK6812_Send();


	}else

            if(string_compare(buffer, "pinon", strlen("pinon")))
	{
			iz=0;
          rz=255,bz=45,gz=51;

	Set_Brightness(25);
	SK6812_Send();

	}
          else
            if(string_compare(buffer, "whion", strlen("whion")))
	{
			iz=0;
          rz=150,bz=150,gz=150;
	Set_Brightness(25);
	SK6812_Send();


	}
          else


// I 점증감식 제어			  
			  
if(string_compare(buffer, "ledon", strlen("ledon")))
     {

while(iz<=44)
{
	iz++;

	Set_Brightness(iz);
	SK6812_Send();

}
   //HAL_UART_Transmit(&huart2, (uint8_t*)"LED is ON.\n", strlen("LED is ON.\n"), 500);
     }
        else
             if(string_compare(buffer, "ledoff", strlen("ledoff")))
     {

 while(iz>=1)
	  {
	iz--;
		  Set_Brightness(iz);
		  SK6812_Send();

	  }
         // HAL_UART_Transmit(&huart2, (uint8_t*)"LED is OFF.\n", strlen("LED is OFF.\n"), 500);
     }
	else
             if(string_compare(buffer, "come", strlen("come")))
     {

       do{
           for (int i=0; i<34; i++)
	  {
		  Set_Brightness(i);
		  SK6812_Send();
//		  HAL_Delay (1);
	  }
              for (int i=35; i>=0; i--)
	  {
		  Set_Brightness(i);
		  SK6812_Send();
		  // HAL_Delay (1);
	  }
       }while(a==50);
     }				
    else
             if(string_compare(buffer, "rain", strlen("rain")))
     {

                      do
			{
				rainbow_effect_left();
				iz = 20;
		//		HAL_Delay (10);
			}
			
			while(a==2);
     }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	for(byte i = 0; i < 160; i++)
	{
		Set_LED(i, rz, gz, bz);
	}

  }	//end main while loop
 
  /* USER CODE END 3 */
}	//end main loop

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 251+1;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 13;
  sTime.Minutes = 27;
  sTime.Seconds = 30;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_FRIDAY;
  sDate.Month = RTC_MONTH_NOVEMBER;
  sDate.Date = 25;
  sDate.Year = 22;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 86-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 720;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BuletoothReset124_GPIO_Port, BuletoothReset124_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_14|BuletoothReset_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BuletoothReset124_Pin */
  GPIO_InitStruct.Pin = BuletoothReset124_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BuletoothReset124_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB14 BuletoothReset_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_14|BuletoothReset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
