/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"

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

/* USER CODE BEGIN PV */
char Rx_data1[2];
char Rx_data5[2];

#define USART1_RX_BUFF_SIZE 100                //GSM
char USART1_Buffer[USART1_RX_BUFF_SIZE];
volatile uint32_t USART1_RX_COUNT = 0;

char *pointer;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#ifdef __GNUC__
	#define putchar_prototype int __io_putchar(int ch)
#else
	#define putchar_prototype int fputc(int ch, FILE *f)
#endif /* __GNUC__*/

putchar_prototype
{
HAL_UART_Transmit(&huart5,(uint8_t*)&ch, 1 , 1000);
return 0;
}

void dbg_Put_Str_P(char *dbgstr)
{
	HAL_UART_Transmit(&huart5,(uint8_t*) dbgstr, strlen(dbgstr), 5000);//used strlen instead of sizeof cause of pointer to string size calculation
}

void Error_led(uint8_t on)
{
	if(on)
	HAL_GPIO_WritePin(LED1_GPIO_Port	, LED1_Pin ,GPIO_PIN_SET);
	else
	HAL_GPIO_WritePin(LED1_GPIO_Port	, LED1_Pin ,GPIO_PIN_RESET);
}
	

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)  
{
		if(huart->Instance == USART1)
		{
			USART1_Buffer[USART1_RX_COUNT] = Rx_data1[0];
			USART1_RX_COUNT++;
//			dbg_Put_Str_P(USART1_Buffer);
			HAL_UART_Receive_IT(&huart1, (uint8_t *)Rx_data1, 1);
		}		
}

void Clear_USART1_Buffer(void)
{
	memset(USART1_Buffer,0,USART1_RX_BUFF_SIZE);
	USART1_RX_COUNT =0;
}

uint8_t Send_AT_RAM(char *command,char *reply,uint32_t reply_timeout)
{
		uint32_t wait_time;
	
	if(reply_timeout == 0	) //no delay and wait for reply i.e. reply is not expected
		{
		HAL_UART_Transmit(&huart1, (uint8_t*)command, strlen(command), 5000);
		return 1;
		}

	wait_time = HAL_GetTick();
	wait_time = wait_time + reply_timeout ;

	Clear_USART1_Buffer();
	pointer = strstr(USART1_Buffer,reply);
	//strcpy_P(aux_str,command) ;
	HAL_UART_Transmit(&huart1, (uint8_t*)command, strlen(command), 5000);
		
	dbg_Put_Str_P((uint8_t*)command);

	while(( wait_time > HAL_GetTick() ) && (!pointer))
		{
		//TODO;
		pointer = strstr(USART1_Buffer,reply);
		//HAL_Delay (100);
		}
		
	dbg_Put_Str_P("RCVD BUFF:");
    dbg_Put_Str_P(USART1_Buffer);
		
	if(!pointer)
	{
	dbg_Put_Str_P(("TIMEUP\r"));
	return 0;
	}
	
	dbg_Put_Str_P(("MATCH\r"));
	return 1;
	
}

uint8_t Init_Modem()
{
//	dbg_Put_Str_P(("ATZ7\r"));			

	if(!Send_AT_RAM("AT\r","OK",2000))
		return 0;
	if(!Send_AT_RAM("ATV1\r","OK",2000))				//Result Codes Numeric Equivalents and Brief Descriptions 
		return 0;
	if(!Send_AT_RAM("ATE0\r","OK",2000))				//Echo OFF
		return 0;
	if(!Send_AT_RAM(" AT+CMEE=2\r","OK",2000))	//Enable result code and use verbose values
		return 0;
	if(!Send_AT_RAM("AT+IPR?\r","OK",2000))			//Get the baudrate, if the value is 0 (auto baudrate),then it will be set to 115200(fixed baudrate)
																							//to assure reliable communication and avoid any problems caused by undetermined 
																							//baudrate between DCE and DTE, value of IPR should be saved with AT&W */

		return 0;
	if(!Send_AT_RAM("ATI\r","OK",2000))					//se ATI to get module information of Manufacturer ID, Device module and Firmware version
		return 0;
	if(!Send_AT_RAM("AT+GSN\r","OK",5000))			//Use AT+GSN to query the IMEI of module
		return 0;
	if(!Send_AT_RAM("AT+CPIN?\r","OK",2000))//Use AT+CPIN? to query the SIM card status : SIM card inserted or not, locked or unlocked
		return 0;	
	
	return 1;
}

uint8_t Reset_Modem(uint8_t retries)
	{
	uint8_t ret =0;
		int i;
//	gsm_power_off();
//	gsm_power_on();
	
	HAL_GPIO_WritePin(GSM_PWR_GPIO_Port, GSM_PWR_Pin, GPIO_PIN_RESET);	///GSM Power OFF
	HAL_Delay(4000);		
	HAL_GPIO_WritePin(GSM_PWR_GPIO_Port, GSM_PWR_Pin, GPIO_PIN_SET);	///GSM Power ON
//	HAL_GPIO_WritePin(GSM_DTR_GPIO_Port, GSM_DTR_Pin, GPIO_PIN_RESET); //WakeUp from Sleep
		
	HAL_Delay(5000);								
	for(i=0;i<retries;i++)
		{
		if(Init_Modem())
			{
			ret =1;
			break;
			}
		else
			{
			//if(!take_scid())	  //no SIM CARD
			//	{
			//	retries = 0;
			//	dbg_Put_Str("SIM ERROR\r");
			//	}

			ret =0;
			}
        }
	
	return(ret);
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_UART5_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_UART_Receive_IT(&huart1, (uint8_t *)Rx_data1, 1);   //activate UART receive interrupt every time	
	HAL_UART_Receive_IT(&huart5, (uint8_t *)Rx_data5, 1);   //activate UART receive interrupt every time
	
printf("HELLO UART5\r\n");
	HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
//	HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
//		printf("HELLO UART5\r\n");
//		HAL_Delay(1000);
		
				uint8_t gprserror =0;
RESETMDM:			
		if(!Reset_Modem(10))
			{
			dbg_Put_Str_P ("Modem Failed...\r");
			Error_led(1);
			goto RESETMDM;
			}
		Error_led(0);	
		dbg_Put_Str_P ("Modem Ready...\r");	
		
		while(1);
		
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

  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
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
