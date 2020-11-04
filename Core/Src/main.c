/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "my_graphic_lib.h"
#include "ssd1306.h"
#include "timers.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

enum item_t{
	TERMINAL,
	BATTERY,
	SIGNAL,
	GRAPH
};

typedef struct {                                // object data type
  uint8_t Buf[32];
  int signal;
  int battery;
  int graph_value;
  enum item_t item;

} MSGQUEUE_OBJ_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

void StartOledTask(void *argument);
void StartUartTask(void *argument);

osThreadId_t oledTaskHandle;
osThreadId_t uartTaskHandle;

const osThreadAttr_t oledTask_attributes = {
  .name = "oledTask",
  .priority = (osPriority_t) osPriorityNormal1,
  .stack_size = 3000
};
const osThreadAttr_t uartTask_attributes = {
  .name = "uartTask",
  .priority = (osPriority_t) osPriorityNormal2,
  .stack_size = 1000
};

osMessageQueueId_t mid_MsgQueue;
osTimerId_t debounce_timer;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

//	static int prescaler = 0;

	 if(GPIO_Pin == GPIO_PIN_0)
	 {
	//	 osTimerStart(debounce_timer, 50U);

		 BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		     if( xTimerStartFromISR( debounce_timer,  &xHigherPriorityTaskWoken ) != pdPASS )
		     {
		         /* The start command was not executed successfully.  Take appropriate
		         action here. */
		     }
		     if( xHigherPriorityTaskWoken != pdFALSE )
		     {
		         /* Call the interrupt safe yield function here (actual function
		         depends on the FreeRTOS port being used). */
		     }
	 }

  /* Prevent unused argument(s) compilation warning */
  ///UNUSED(GPIO_Pin);
  /* NOTE: This function Should not be modified, when the callback is needed,the HAL_GPIO_EXTI_Callback could be implemented in the user file */
}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	MSGQUEUE_OBJ_t g_data;

	uint16_t adc_value = hadc->Instance->DR;

	g_data.item = GRAPH;
	g_data.graph_value = adc_value;
    osMessageQueuePut(mid_MsgQueue, &g_data, 0U, 0U);

    return;

//	uint8_t buf[3];
//
//	buf[0] = 0xf7;
//	buf[1] = adc_value>>8;
//	buf[2] = adc_value;
//
//
//	HAL_UART_Transmit( &huart2,  buf, 3, 1000);
//

//	adcv[adci] = adc_value;
//	adci++;
//	if(adci >= 1024) adci = 0;
  /* Prevent unused argument(s) compilation warning */
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_ADC_ConvCpltCallback could be implemented in the user file
   */
}

static void debounce_timer_Callback (void *argument) {
  int32_t arg = (int32_t)argument; // cast back argument '0'
  // do something, i.e. set thread/event flags

     static int prescaler = 0;

	 if(prescaler == 0) {
		 TIM8->PSC = 84;	//100Hz
	 }
	 if(prescaler == 1) {
		 TIM8->PSC = 84*2;	//50Hz
	 }
	 if(prescaler == 2) {
		 TIM8->PSC = 84*3; //30Hz
	 }
	 if(prescaler == 3) {
		 TIM8->PSC = 84*4; //25Hz
	 }

	 prescaler++;
	 if(prescaler >= 4) prescaler = 0;


	 return;


  MSGQUEUE_OBJ_t tx_data;

	static int signal = 0;


/*	 tx_data.item = TERMINAL;
	 sprintf(tx_data.Buf, "Button pressed");
	 osMessageQueuePut(mid_MsgQueue, &tx_data, 0U, 0U);    */


	tx_data.item = SIGNAL;
		tx_data.signal = signal++;
	    osMessageQueuePut(mid_MsgQueue, &tx_data, 0U, 0U);
	    if(signal > 5) signal = 0;

}



void StartOledTask(void *argument)
{

  char* lines[5];
  char bat[5];
  char signal[5];


  HAL_TIM_Base_Start(&htim8);
  HAL_ADC_Start_IT(&hadc1);


  memset(bat, 0,  sizeof(bat));
  memset(signal, 0, sizeof(signal));



  for( int i = 0; i < 5; i++ ) {
	  lines[i] = pvPortMalloc(32);
	  memset(lines[i], 0, 32);
  }



  osTimerStart(debounce_timer, 400U);

  uint8_t graph[128];
  memset(graph, 0, 128);

  //////////////
  while(1)
  {
	MSGQUEUE_OBJ_t data;

	if (osMessageQueueGet(mid_MsgQueue, &data, NULL, 1000U) == osOK) {

		if(data.item == TERMINAL) {
			for( int i = 0; i < 5; i++ ) {
				print_string(lines[i], 1, 9*2 + i*9, 0);
			}

			for(int i  = 0; i < 4; i++ ) {
				memcpy(lines[i], lines[i + 1], 32);
			}
			memcpy(lines[4], data.Buf, 32);

			for( int i = 0; i < 5; i++ ) {
				print_string(lines[i], 1, 9*2 + i*9, 1);
			}
		}

		else if( data.item == BATTERY ) {
			print_string(bat, 1, 1 , 0);
			sprintf(bat, "%d%%", data.battery);
			print_string(bat, 1, 1 , 1);
		}

		else if( data.item == GRAPH ) {
			// clean the graph pixels from the oled
			for(int i = 0; i < 127; i++) {
				drawline(i, graph[i], i+1 ,graph[i+1], 0);
				//set_pixel(i, graph[i], 0);
			}

			//move entire graph pixels to left
			for(int i = 0; i < 127; i++) {
				graph[i] = graph[i+1];
			}

			// graph variable should have value between 0 to 63 only because it is y value.
			//2048 to 2200 is range
			int adc = data.graph_value/2;
			adc -= 2040/2;
			if(adc < 0) adc = 0;
			if(adc > 63) adc = 63;

			graph[127] = (uint8_t) adc;

			// clean the graph pixels from the oled
			for(int i = 0; i < 127; i++) {
				drawline(i, graph[i], i+1 ,graph[i+1], 1);
				//set_pixel(i, graph[i], 0);
			}

//			for(int i = 0; i < 128; i++) {
//				set_pixel(i, graph[i], 1);
//			}
		}

		else if( data.item == SIGNAL ) {
			print_string(signal, 128 - 6*2, 1 , 0);
			sprintf(signal, "%d", data.signal);
			print_string(signal, 128 - 6*2, 1 , 1);
		}

		ssd1306_UpdateScreen();

	}


//	char buff[20];
//	sprintf(buff, "%d", count++);
//	print_string( buff, 10, 10, 1 );
//	ssd1306_UpdateScreen();
//    osDelay(1000);
//	print_string( buff, 10, 10, 0 );
  }
  /* USER CODE END 5 */
}

void StartUartTask(void *argument)
{
  int count = 0;

  int battery, signal;

  battery = 0;
  signal = 0;

  for(;;)
  {
/*	MSGQUEUE_OBJ_t tx_data;

	tx_data.item = TERMINAL;
	int free_heap =  xPortGetFreeHeapSize();
	sprintf(tx_data.Buf, "Free heap %d KB", free_heap/1024);
    osMessageQueuePut(mid_MsgQueue, &tx_data, 0U, 0U);

	tx_data.item = BATTERY;
	tx_data.battery = battery++;
    osMessageQueuePut(mid_MsgQueue, &tx_data, 0U, 0U);
    if(battery > 100) battery = 0;
*/
/*	tx_data.item = SIGNAL;
	tx_data.signal = signal++;
    osMessageQueuePut(mid_MsgQueue, &tx_data, 0U, 0U);
    if(signal > 5) signal = 0;   */

    osDelay(500);

  }
  /* USER CODE END 5 */
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in freertos.c) */
 // MX_FREERTOS_Init();

  /* Start scheduler */
  //osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

   mid_MsgQueue = osMessageQueueNew(30, sizeof(MSGQUEUE_OBJ_t), NULL);
   if (mid_MsgQueue == NULL) {
     ; // Message Queue object not created, handle failure
   }
   debounce_timer = osTimerNew(debounce_timer_Callback, osTimerOnce, (void *)0, NULL);     // (void*)0 is passed as an argument


  oledTaskHandle = osThreadNew(StartOledTask, NULL, &oledTask_attributes);
  uartTaskHandle = osThreadNew(StartUartTask, NULL, &uartTask_attributes);


   ssd1306_Init();	//initialize the oled display for 128 x 64 pixels

   memset(SSD1306_Buffer, 0x0, 128 * 8);	//clear the display buffer


   osKernelStart();


  while (1)
  {
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

	//    object_t square_obj;
	//    object_t square2_obj;
	//
	//    square_obj.object_type = OBJECT_SQUARE;
	//    square_obj.square.x1 = 0;
	//    square_obj.square.y1 = 0;
	//    square_obj.square.x2 = 20;
	//    square_obj.square.y2 = 20;
	//    square_obj.square.fill = 0;
	//
	//    square2_obj.object_type = OBJECT_SQUARE;
	//    square2_obj.square.x1 = 0;
	//    square2_obj.square.y1 = 64/2;
	//    square2_obj.square.x2 = 0 + 10;
	//    square2_obj.square.y2 = 64/2 + 10;
	//    square2_obj.square.fill = 0;
	//
	//    object_t circle1_obj;
	//    object_t circle2_obj;
	//
	//    circle1_obj.object_type = OBJECT_CIRCLE;
	//    circle1_obj.circle.x1 = 10;
	//    circle1_obj.circle.y1 = 10;
	//    circle1_obj.circle.radius = 5;
	//    circle1_obj.circle.fill = 0;
	//
	//    circle2_obj.object_type = OBJECT_CIRCLE;
	//    circle2_obj.circle.x1 = 10;
	//    circle2_obj.circle.y1 = 20;
	//    circle2_obj.circle.radius = 15;
	//    circle2_obj.circle.fill = 0;
	//
	//
	//    object_init(&square_obj);
	//    object_init(&square2_obj);
	//    object_init(&circle1_obj);
	//    object_init(&circle2_obj);
	//
	//    while(1)
	//    {
	//  	  for(int i = 0; i < 64; i++) {
	//  		  object_move(&square_obj, 1, 1);
	//  		  object_move(&square2_obj, 1, 0);
	//
	//  		  object_move(&circle1_obj, 1, 1);
	//  		  object_move(&circle2_obj, 1, 0);
	//
	//  		  HAL_Delay(100);
	//  	  }
	//
	//  	  for(int i = 0; i < 64; i++) {
	//  	  	   object_move(&square_obj, -1, -1);
	//  	  	   object_move(&square2_obj, -1, 0);
	//  	  	   object_move(&circle1_obj, -1, -1);
	//  	  	   object_move(&circle2_obj, -1, 0);
	//
	//  	  		HAL_Delay(100);
	//  	  }
	//
	//    }

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
