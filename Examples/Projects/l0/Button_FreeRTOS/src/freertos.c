/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "luos.h"
#include "button.h"
#include "led.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for LuosTask */
const osThreadAttr_t LuosTask_attributes = {
    .name       = "LuosTask",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t ButtonTask_attributes = {
    .name       = "Button",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t LedTask_attributes = {
    .name       = "Led",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartLuosTask(void *argument);
void StartButtonTask(void *argument);
void StartLedTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */

    /* creation of LuosTask */
    Luos_Init();
    Button_Init();
    Led_Init();

    osThreadNew(StartLuosTask, NULL, &LuosTask_attributes);
    osThreadNew(StartButtonTask, NULL, &ButtonTask_attributes);
    osThreadNew(StartLedTask, NULL, &LedTask_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartLuosTask */
/**
 * @brief Function implementing the LuosTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartLuosTask */
void StartLuosTask(void *argument)
{
    /* USER CODE BEGIN StartLuosTask */
    /* Infinite loop */
    while (1)
    {
        Luos_Loop();

        taskYIELD();
    }
    /* USER CODE END StartLuosTask */
}

void StartButtonTask(void *argument)
{
    /* USER CODE BEGIN StartButtonTask */
    /* Infinite loop */
    while (1)
    {
        Button_Loop();

        taskYIELD();
    }
    /* USER CODE END StartButtonTask */
}

void StartLedTask(void *argument)
{
    /* USER CODE BEGIN StartLedTask */
    /* Infinite loop */
    while (1)
    {
        Led_Loop();

        taskYIELD();
    }
    /* USER CODE END StartLedTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
