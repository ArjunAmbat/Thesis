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
//#define V_REF 6.0f
//#define V_ADC 3.3f
//#define ADC_RESOL 4095.0f
//#define R1 100000.0f
//#define R2 5100.0f
//#define R3 3900.0f
//#define R4 6200.0f
//#define GAIN_SEC 8.2f
//#define OFFSET_V 1.65f // 0A point
//#define GAIN_PRInR 0.8f // 0.04 * 20
//#define GAIN_SECnR  0.164f   // 0.025 * 8.2 * 0.8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
volatile uint16_t ADC1_Data[1];
volatile uint16_t ADC2_Data[1];
volatile float V_PRI_ADC, I_PRI_ADC, V_SEC_ADC, I_SEC_ADC, V_PRI, I_PRI, V_SEC, I_SEC, V_SEC_f;
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

volatile uint32_t COMP1_state = 0;
volatile float V_thr = 0;
volatile uint32_t dac_code = 0;

//volatile uint32_t COMP1_state = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
//volatile uint16_t ADC1_Data[1];
//volatile uint16_t ADC2_Data[1];
//volatile float V_PRI_ADC, I_PRI_ADC, V_SEC_ADC, I_SEC_ADC, V_PRI, I_PRI, V_SEC, I_SEC;
float i_ref = 0.1f;
float i_max = 1; //1A
float V_SEC_FILTERED;
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

float Kp = 0.05f;	// change
float Ki = 200.0f;  // change
volatile float v_error = 0.0f;
volatile float integral = 0.0f;
volatile float duty = 0.05f;
#define Tc 0.00004f   // 40 us (25 kHz)
extern DAC_HandleTypeDef hdac1;

volatile float V_REF = 6.1620f; // Target output voltage


//#define V_DROP  0.2f
//float V_REF   = V_SET + V_DROP;
volatile uint32_t CMP1_NEW = 10U;
float I_PEAK_LIM = 1.0f;   // Amps; change this to set the limit

void HRTIM1_TIMA_IRQHandler(void)
{
	HRTIM1_TIMA->TIMxICR |= (1U<<4);

	// ----- Update PCM peak current threshold -----
    V_thr = 1.65f + I_PEAK_LIM * 0.8f;
    dac_code = (uint32_t)((V_thr / 3.3f) * 4095.0f);

    // Set DAC value
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_code);

    COMP1_state = (COMP1->CSR >> 30) & 1U;

	I_PRI_ADC = (ADC2_Data[0]*3.3)/4095;
	I_PRI = (I_PRI_ADC-1.65)/0.8; /*VSHUNT​=I*R=I*0.04; VAMP​=I*0.04*20=I*0.8; VADC​=1.65+I*0.8; SOLVE I */

	V_SEC_ADC = (ADC1_Data[0]*V_ADC)/ADC_RESOL;
	V_SEC = V_SEC_ADC * ((R3 + R4) / R4) / (0.4F * 1.6F);

	// --- Filter ---
	V_SEC_f = 0.70f * V_SEC_f + 0.30f * V_SEC;

	static uint8_t count = 0;

    if (++count >= 4)  // until 25kHz Control loop
    {

    count = 0;

    // ---- Voltage PI Controller ----

    v_error = V_REF - V_SEC_f;

    integral += v_error * Tc;

    // Anti-windup clamp on integral
    if (integral >  10.0f)
    	{
    	integral =  10.0f;
    	}
    if (integral < 0.0f)
    	{
    	integral = 0.0f;
    	}

    duty = Kp * v_error + Ki * integral;

    // Clamp duty cycle output
    if (duty < 0.01f)
    	{
    	duty = 0.01f;
    	}
    if (duty > 0.45f)
    	{
    	duty = 0.45f;
    	}

    CMP1_NEW = (uint32_t)(duty * (float)HRTIM1_TIMA->PERxR);

 //Corner frequency

    HRTIM1_TIMA->CMP1xR = CMP1_NEW;


    }
//	//-----Read voltage and current values (run at 100kHz)-----
//	I_PRI_ADC = (ADC2_Data[0]*3.3)/4095;
//	I_PRI = (I_PRI_ADC-1.65)/0.8; /*Vshunt​=I*R=I*0.04; Vamp​=I*0.04*20=I*0.8; VADC​=1.65+I*0.8; Solve I */
//
//	V_SEC_ADC = (ADC1_Data[0]*V_ADC)/ADC_RESOL;
//	V_SEC = V_SEC_ADC * ((R3 + R4) / R4) / (0.4f * 1.6f);
//
//    // ----- DIGITAL FILTER (100 kHz) -----
//    static float V_SEC_f = 0.0f;
//    V_SEC_f = 0.98f * V_SEC_f + 0.02f * V_SEC;
//
//    V_SEC_FILTERED = V_SEC_f;

//	I_SEC_ADC = (ADC1_Data[1]*V_ADC)/ADC_RESOL;
//	I_SEC = (I_SEC_ADC-OFFSET_V)/GAIN_SECnR; //-ve
//
//	V_PRI_ADC = (ADC2_Data[1]*V_ADC)/ADC_RESOL;
//	V_PRI  = V_PRI_ADC * ((R1 + R2) / R2);
//    }

}



/* USER CODE END 1 */
