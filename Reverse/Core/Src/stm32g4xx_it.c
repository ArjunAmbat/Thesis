/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define V_ADC 3.3f
#define ADC_RESOL 4095.0f
#define R1 100000.0f
#define R2 5100.0f
#define R3 3900.0f
#define R4 6200.0f
#define GAIN_SEC 8.2f
#define OFFSET_V 1.65f // 0A point
#define GAIN_PRInR 0.8f // 0.04 * 20
#define GAIN_SECnR  0.164f   // 0.025 * 8.2 * 0.8
#define Tc 0.0001f   // 100 us (10 kHz)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint16_t ADC1_Data[1];
volatile uint16_t ADC2_Data[1];
volatile float V_PRI_ADC, I_PRI_ADC, V_SEC_ADC, I_SEC_ADC, V_PRI_f, V_PRI, I_PRI, V_SEC, V_SEC_f, I_SEC, I_SEC_f, I_SEC_g;
float i_ref = 0.1f;
float i_max = 1; //1A
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g4xx.s).                    */
/******************************************************************************/


/* USER CODE BEGIN 1 */

volatile float Kp_v = 0.5;
volatile float Ki_v = 20.0f;

volatile float Kp_i = 0.5f;
volatile float Ki_i = 200.0f;

//volatile float Kp_v = 0.01;
//volatile float Ki_v = 0.001f;
//
//volatile float Kp_i = 0.01f;
//volatile float Ki_i = 600.0f;

volatile float v_error = 0.0f;
volatile float i_error = 0.0f;

volatile float integral_v = 0.0f;
volatile float integral_i = 0.0f;

volatile float duty = 0.05f;

#define Tc_v 0.00025f   // 250 us (4 kHz) outer voltage loop
#define Tc_i 0.00004f  // 40 us (25 kHz) inner current loop

volatile float V_REF = 3.0f; // Target sink voltage
volatile float I_REF = 0.1f; // Target sink current

volatile uint32_t CMP1_NEW = 10U;

void HRTIM1_TIMA_IRQHandler(void)
{
	HRTIM1_TIMA->TIMxICR |= (1U<<4);
	static uint8_t i_count = 0;
	static uint8_t v_count = 0;

	I_SEC_ADC = (ADC1_Data[0]*V_ADC)/ADC_RESOL;
	I_SEC = (I_SEC_ADC-OFFSET_V)/GAIN_SECnR; //-ve

	V_PRI_ADC = (ADC2_Data[0]*V_ADC)/ADC_RESOL;
	V_PRI  = V_PRI_ADC * ((R1 + R2) / R2);

	V_SEC_ADC = (ADC1_Data[1]*V_ADC)/ADC_RESOL;
	V_SEC = V_SEC_ADC * ((R3 + R4) / R4) / (0.4F * 1.6F);

	// --- Filter ---
	I_SEC_f = 0.995f * I_SEC_f + 0.005f * I_SEC;

	// --Gain--
	I_SEC_g = I_SEC_f - 0.350;

	// --- Filter ---
	V_SEC_f = 0.90f * V_SEC_f + 0.10f * V_SEC;

    if (++i_count >= 4)  // 20kHz Inner Current Control loop
    {

    i_count = 0;

    // ---- Voltage PI Controller ----

    i_error = I_REF - I_SEC_g;

    integral_i += i_error * Tc_i;

    // Anti-windup clamp on integral
    if (integral_i >  10.0f)
    	{
    	integral_i =  10.0f;
    	}
    if (integral_i < 0.0f)
    	{
    	integral_i = 0.0f;
    	}

    duty = Kp_i * i_error + Ki_i * integral_i;

    // Clamp duty cycle output
    if (duty < 0.01f)
    	{
    	duty = 0.01f;
    	}
    if (duty > 0.98f)
    	{
    	duty = 0.98f;
    	}

    CMP1_NEW = (uint32_t)(duty * (float)HRTIM1_TIMA->PERxR);

    HRTIM1_TIMA->CMP1xR = CMP1_NEW;

    }

    if (++v_count >= 25)  // 4kHz Outer Voltage Control loop
    {
    // ---- Voltage PI Controller ----
    v_count = 0;

    v_error = V_SEC_f - V_REF;

    integral_v += v_error * Tc_v;

    // Anti-windup clamp on integral
    if (integral_v > 10.0f)
        integral_v = 10.0f;

    if (integral_v < 0.0f)
        integral_v = 0.0f;

    I_REF = Kp_v * v_error + Ki_v * integral_v;

    // Clamp duty cycle output
    if (I_REF < 0.0f)
    	{
    	I_REF = 0.0f;
    	}
    if (I_REF > 1.5f)
    	{
    	I_REF = 1.5f;   // Reference i never goes above 1A
    	}
    }

}



/* USER CODE END 1 */
