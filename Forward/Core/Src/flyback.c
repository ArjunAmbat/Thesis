/*
	 PIN MAP

	 -------------------------------------------------------------

	 PA1 - ADC2_IN2 - primary current - ADC2_data[0] - Pin 13 - COMP1_INP
	 PA2 - ADC1_IN3 - secondary voltage - ADC1_data0] - Pin 14

	 PA3 - ADC1_IN4 - secondary current - ADC1_data[1] - Pin 17 - COMP2_INP
	 PA0 - ADC2_IN1 - primary voltage - ADC2_data[1] - Pin 12

	 -------------------------------------------------------------

	 PWMA - HRTIM_CHA1 - PA8 - Pin 42
	 PWMB - HRTIM_CHA2 - PA9 - Pin 43

	 -------------------------------------------------------------

	 M1 Signal - PA10 - Pin 44
	 M2 Signal - PA11 - Pin 45

	 -------------------------------------------------------------


*/

#include "main.h"
#include "stm32g4xx_hal.h"



//-------- HRTIM CONFIG --------
void HRTIM_CONFIG (void)
{

  //-------- HRTIM1 CONFIG --------

  //Clock enable
  RCC->APB2ENR |= (1U<<26);

  //Continuous mode, Pre-load of reg enable, update at reset enable, clock pre-scaler 100 for 170*2=340MHz=>2.94ns resolution selection
  HRTIM1_TIMA->TIMxCR |= (1U<<3);//CONT; Timer operates in continuous mode

  //PREEN; Write access is done into the pre-load register
  /*When PREEN = 0 (Disabled): As soon as you write a value to a register (like CMP1AR), it immediately changes the hardware's behavior.
   *If you change a value right as the counter is hitting it, you might get a "glitch" or a missing PWM pulse.
   *When PREEN = 1 (Enabled): When you write a new value, it sits quietly in the Preload register. It does not affect the timer yet.
   *The value is only copied into the Active register when a specific "Update Event" occurs (usually when the timer rolls over to zero
   *or a specific reset trigger happens).*/

  HRTIM1_TIMA->TIMxCR |= (1U<<27);

  //Register update is triggered by timer x counter reset/roll-over
  HRTIM1_TIMA->TIMxCR |= (1U<<18);

  //F_HRCK= 170Mhz; HRCK pre-scaler (100) ratio 170*2=340MHz and 2.94ns resolution
  HRTIM1_TIMA->TIMxCR |= (1U<<2);
  HRTIM1_TIMA->TIMxCR &= ~(1U<<1);
  HRTIM1_TIMA->TIMxCR &= ~(1U<<0);

  // Setting period for 100KHz
  HRTIM1_TIMA->PERxR = 3399U; //PER = fHRCK / fPWM; 340MHz=2.94ns and 100KHz=10us=> 10us/2.94ns=3401. 1 values less than because counting from 0

  //Setting a duty ratio
  HRTIM1_TIMA->CMP1xR = 100U; // 1481

  if (HRTIM1_TIMA->CMP1xR >= 1500U)
  {
	  HRTIM1_TIMA->CMP1xR >= 1500U;
  }

  //Setting output 1 high at the beginning of PWM cycle (PER)
  HRTIM1_TIMA->SETx1R |= (1U<<2);

  //Resetting output 1 low when the counter=CMP1
  HRTIM1_TIMA->RSTx1R |= (1U<<3);

  //Setting and resetting output 2 is automatic with dead time enable

  	  	  	  //----- HRTIM INTERRUPT CONFIG -----
  	  	  	  /* We are now interrupting the processor by this statement, if interrupt the processor and dont put an interrupt service routine
   	   	   	   then it will go into an infinite wait loop. So we want to clear the flag.*/

  	  	  	  //Interrupt at period
  	  	  	  HRTIM1_TIMA->CMP2xR = HRTIM1_TIMA->CMP1xR + 300U; //Set CMP2 to match the Period
  	  	  	  HRTIM1_TIMA->TIMxDIER |= (1U<<1); //Enable the CMP2 Interrupt i.e Period

  	  	  	  //NVIC
  	  	  	  HAL_NVIC_SetPriority(HRTIM1_TIMA_IRQn, 0, 0);
  	  	  	  HAL_NVIC_EnableIRQ(HRTIM1_TIMA_IRQn);

  //Enable dead-time
  HRTIM1_TIMA->OUTxR |= (1U<<8);

  //Specifying dead-time and dead time pre-scaler (170MHz)
  //PriMOS tr=10ns, td(on)=10ns; tf=8ns, td(off)=22ns =>DTF = tf+td(off)-td(on,secMOS) = 30ns-14ns=16ns=>35ns
  //SecMOS tr=16ns, td(on)=14ns; tf=8ns, td(off)=33ns =>DTR = tf+td(off)-td(on,priMOS) = 41ns-10ns=31ns=>50ns
  //We need 35ns and 50ns of dead time; so if we use tDTG of 2.94ns (010) we get 12 and 17 as values
  /* Timer and the Dead-time Unit are two different "rooms" inside the same building. They don't have to use the same clock speed.
   * t_DTG stands for Dead-time Generator clock. If t_DTG = 2.94ns, then every 1 you write in the register adds 2.94ns of delay.*/
  HRTIM1_TIMA->DTxR =  (2U << 10) | (12U << 16) | (17U << 0);

  //Enable timer A1 and A2
  /*It basically "connects" the internal PWM signal to the physical GPIO pin.*/
  HRTIM1_COMMON->OENR |= (1U<<0);
  HRTIM1_COMMON->OENR |= (1U<<1);


  //----- GPIO CONFIG -----
  //Configuring PA8 and PA9 as HRTIM CHA1 and CHA2

  //Enable GPIOA
  //Clock is already enabled

  //Set to Alternate Function Mode
  GPIOA->MODER |= (1U<<17);
  GPIOA->MODER &= ~(1U<<16);
  GPIOA->MODER |= (1U<<19);
  GPIOA->MODER &= ~(1U<<18);

  //Alternate function number AF13
  GPIOA->AFR[1] |= (1U<<3);
  GPIOA->AFR[1] |= (1U<<2);
  GPIOA->AFR[1] &= ~(1U<<1);
  GPIOA->AFR[1] |= (1U<<0);

  GPIOA->AFR[1] |= (1U<<7);
  GPIOA->AFR[1] |= (1U<<6);
  GPIOA->AFR[1] &= ~(1U<<5);
  GPIOA->AFR[1] |= (1U<<4);

  //Output speed to very high
  GPIOA->OSPEEDR |= (1U<<17);
  GPIOA->OSPEEDR |= (1U<<16);

  GPIOA->OSPEEDR |= (1U<<19);
  GPIOA->OSPEEDR |= (1U<<18);

}


//Peak Current Mode current limit
void PCM_LIMIT_CONFIG(void)
{
	// COMP1 is on hrtim_eev4 SRC2(01)
	// Table 223. External events mapping and associated features
	HRTIM1_COMMON->EECR1 &= ~(1U<<19); //EE4SRC
	HRTIM1_COMMON->EECR1 |= (1U<<18);

	// EE4POLL = 0 active high (COMP1 non-inverted: 1 when I_PRI > I_ref)

	//External event 4 sensitivity 01: Rising edge, whatever EE4POL bit value
	HRTIM1_COMMON->EECR1 &= ~(1U<<22); //EE4SNS
	HRTIM1_COMMON->EECR1 |= (1U<<21);

	//External event 4 fast mode
	HRTIM1_COMMON->EECR1 &= ~(1U<<23); //EE4FAST LATER CHANGE IT TO 1 (low latency mode)

	//EEV4 Leading edge blanking
	//0011: Blanking from counter reset/roll-over to compare 3
	// EEV4 is IGNORED inside this window -> kills turn-on ringing spike.
	HRTIM1_TIMA->EEFxR1 &= ~(1U<<22);
	HRTIM1_TIMA->EEFxR1 &= ~(1U<<21);
	HRTIM1_TIMA->EEFxR1 |= (1U<<20);
	HRTIM1_TIMA->EEFxR1 |= (1U<<19);

	// Blanking time = CMP3 * tHRCK ; tHRCK ~= 2.94 ns
	// Start ~200 ns: 200 ns / 2.94 ns ~ 68 ticks.
	HRTIM1_TIMA->CMP3xR = 68U;

    //----- Add EEV4 as an extra reset source for TA1 -----
    HRTIM1_TIMA->RSTx1R |= (1U << 24);     // EXTEVNT4 -> reset TA1

}


//----- ADC CONFIG -----
void ADC_CONFIG (void)
{

  //----- GPIO PA0, PA1, PA2 and PA3 CONFIG -----
  //Clock is already enabled

  // -------- GPIO Analog Mode --------
  GPIOA->MODER |= (1U<<0);
  GPIOA->MODER |= (1U<<1);

  GPIOA->MODER |= (1U<<2);
  GPIOA->MODER |= (1U<<3);

  GPIOA->MODER |= (1U<<4);
  GPIOA->MODER |= (1U<<5);

  GPIOA->MODER |= (1U<<6);
  GPIOA->MODER |= (1U<<7);

  // -------- ADC CLOCK --------
  //Enable ADC12 clock
  RCC->AHB2ENR |= (1U<<13);

  /* ADC module supplied by two possible clocks, Sysclk or PLL P clk. By default Async clk (PLLP).*/
  // Select SysClk to be input clock: 01
  ADC12_COMMON->CCR &= ~(1U<<17);
  ADC12_COMMON->CCR |= (1U<<16);

  // Pre-scaler: divide by 4
  ADC12_COMMON->CCR &= ~(1U<<21); //ADC_CLK = SysClk/4 =170Mhz/4 = 42.5Mhz
  ADC12_COMMON->CCR &= ~(1U<<20);
  ADC12_COMMON->CCR |= (1U<<19);
  ADC12_COMMON->CCR &= ~(1U<<18);

  // 12-bit resolution (ADC_CFGR)

  // Right data alignment (ADC_CFGR)

  // Regular ADC operating mode (Gain compensation mode = 0) (ADC_CFGR2)
  /*A special feature of the STM32G4 ADC that allows you to digitally "trim" or amplify the
   result to correct for hardware errors. Setting it to 0 means you are using the raw,
   unadjusted conversion value.*/

  // -------- ADC POWER UP --------
  // ADC exit deep power down
  ADC1->CR &= ~(1U<<29); //0 = ADC not in Deep-power down
  ADC2->CR &= ~(1U<<29);

  // Enable regulator
  ADC1->CR |= (1U<<28);
  ADC2->CR |= (1U<<28);

  // Wait for regulator stabilization (>2us)
  /*For creating a delay, the CPU doesn't care what the ADC's internal prescaler is doing; it only knows its own frequency.
    Since the CPU is running at fhclk,here 170Mhz, the ticks are based on that higher speed.*/
  // 1/170MHz= 5.88ns; 2us/5.88ns = 340
  for (volatile int i = 0; i < 10000; i++);

  // -------- CALIBRATION --------
  // Single-ended calibration; 0:single ended; 1:differential
  ADC1->CR &= ~(1U<<30);
  ADC2->CR &= ~(1U<<30);

  // Start calibration; Hardware will clear the bit once the calibration is complete
  ADC1->CR |= (1U<<31);
  ADC2->CR |= (1U<<31);

  // Wait for calibration to complete
  while ((ADC1->CR & (1U<<31)) != 0) {}
  while ((ADC2->CR & (1U<<31)) != 0) {}

  // -------- SEQUENCE CONFIG --------
  //ADC sequence length: 1 (for each ADC1 and ADC2)
  //Once we trigger conversion how many channels should it convert.
  ADC1->SQR1 &= ~(1U<<3);
  ADC1->SQR1 &= ~(1U<<2);
  ADC1->SQR1 &= ~(1U<<1);
  ADC1->SQR1 &= ~(1U<<0);

  ADC2->SQR1 &= ~(1U<<3);
  ADC2->SQR1 &= ~(1U<<2);
  ADC2->SQR1 &= ~(1U<<1);
  ADC2->SQR1 &= ~(1U<<0);

  // "Scan" mode is disabled as sequence length 1. It is used when you want to convert a list of multiple channels
  // By disabling it, you are telling the ADC to focus on just one single channel in its sequence.

  //ADC conversion sequence order
  //Write the channel number(not ADC number) explicitly
  //First priority (SQ1)
  //ADC2_IN2 => Primary current
  ADC2->SQR1 &= ~(1U<<10);
  ADC2->SQR1 &= ~(1U<<9);
  ADC2->SQR1 &= ~(1U<<8);
  ADC2->SQR1 |= (1U<<7);
  ADC2->SQR1 &= ~(1U<<6);

  //ADC1_IN3 => Secondary voltage
  ADC1->SQR1 &= ~(1U<<10);
  ADC1->SQR1 &= ~(1U<<9);
  ADC1->SQR1 &= ~(1U<<8);
  ADC1->SQR1 |= (1U<<7);
  ADC1->SQR1 |= (1U<<6);


  // -------- SAMPLING TIME --------
  //Selecting the sampling time of the channels; here two channels so we use SMP0 and SMP1 in SMPR1 reg
  /* When sampling command is given, it will close a switch. The analog signal fed to analog pin will now charge a capacitor in
     the sample and hold circuit. The longer you keep the switch closed, the longer the capacitor will be charged and closer the
     cap voltage will be to the analog signal. But longer we keep the switch closed, the more time we are taking for the conversion.
     This reduces the total time left for the control. (Here 6.5ADC clock cycles are used. i.e 001)
     Here 6.5 Clock cycles means 170MHz/4= 42.5MHz => 6* (1/42.5MHz) = 0.152us. This is multiplied by how many samples we require.

     PA0 is ADC2_IN1 - primary voltage - SMP1[2:0]
	 PA1 is ADC2_IN2 - primary current - SMP2[2:0] - COMP1_INP
	 PA2 is ADC1_IN3 - secondary voltage - SMP4[2:0]
	 PA3 is ADC1_IN4 - secondary current - SMP5[2:0] - COMP2_INP

  */

  ADC2->SMPR1 &= ~(1U<<8); //SMP2[2:0] for primary current in ADC2_IN2
  ADC2->SMPR1 &= ~(1U<<7);
  ADC2->SMPR1 &= ~(1U<<6);

  ADC1->SMPR1 &= ~(1U<<11); //SMP3[2:0] for secondary voltage in ADC1_IN3
  ADC1->SMPR1 &= ~(1U<<10);
  ADC1->SMPR1 &= ~(1U<<9);

//  ADC1->SMPR1 &= ~(1U<<14); //SMP4[2:0] for secondary current in ADC1_IN4
//  ADC1->SMPR1 &= ~(1U<<13);
//  ADC1->SMPR1 |= (1U<<12);
//
//  ADC2->SMPR1 &= ~(1U<<5); //SMP1[2:0] for primary voltage in ADC2_IN1
//  ADC2->SMPR1 &= ~(1U<<4);
//  ADC2->SMPR1 |= (1U<<3);

  // -------- ADC CONFIG --------
  //Configure ADC1 and ADC2 for DMA1
  //DMA1 enable
  ADC1->CFGR |= (1U << 0);
  ADC2->CFGR |= (1U << 0);

  //DMA1 Circular Mode - It tells the ADC to keep generating DMA "requests" indefinitely.
  ADC1->CFGR |= (1U << 1);
  ADC2->CFGR |= (1U << 1);

  //Discontinuous mode disable (DISCNUM 000 = 1 channel)
  ADC1->CFGR &= ~(1U<<16);
  ADC2->CFGR &= ~(1U<<16);

  /*If you had a long list of channels, this would let you break them into small groups (e.g., convert 2 channels per trigger).
   * Since you only have 1 channel, this is not needed.*/

//  // --- Continuous mode ---
  ADC1->CFGR |= (1U << 13);
  ADC2->CFGR |= (1U << 13);

//  // DISABLE Continuous mode
//  ADC1->CFGR &= ~(1U << 13);
//  ADC2->CFGR &= ~(1U << 13);

  /*ContinuousConvMode = DISABLE
Enabled: Once triggered, the ADC converts forever in a loop.
Disabled: The ADC converts once per HRTIM trigger.*/

  // EOCIE not enabled

  ADC1->CFGR |= (1U<<12); // OVRMOD
  ADC2->CFGR |= (1U<<12);

//  // --- External trigger enable and polarity selection for regular channels ---
//  ADC1->CFGR &= ~(1U << 11); // EXTEN = 01 (Hardware trigger detection on the rising edge)
//  ADC1->CFGR |= (1U << 10);
//
//  ADC2->CFGR &= ~(1U << 11); // EXTEN = 01 (Hardware trigger detection on the rising edge)
//  ADC2->CFGR |= (1U << 10);


//  /*ONE trigger source (TRG1)
//	BUT triggered at TWO moments:
//    - RESET
//    - PERIOD*/
//  //Configuring ADC1 to listen to HRTIM_ADC_TRG1 (Check the table)
//  //Generated by HRTIMA reset or roll-over on SQ1, HRTIM period for SEQ2
//  ADC1->CFGR |= (1U<<9);
//  ADC1->CFGR &= ~(1U<<8);
//  ADC1->CFGR |= (1U<<7);
//  ADC1->CFGR &= ~(1U<<6);
//  ADC1->CFGR |= (1U<<5);
//  //Configuring ADC2 to listen to HRTIM_ADC_TRG1 (Check the table)
//  //Generated by HRTIMA reset or roll-over on SQ1, HRTIM period for SEQ2
//  ADC2->CFGR |= (1U<<9);
//  ADC2->CFGR &= ~(1U<<8);
//  ADC2->CFGR |= (1U<<7);
//  ADC2->CFGR &= ~(1U<<6);
//  ADC2->CFGR |= (1U<<5);

//  // 1. Clear EXTSEL bits (bits 9:5) and EXTEN bits (bits 11:10)
//  ADC1->CFGR &= ~(0x3F << 5);
//
//  // 2. Set EXTSEL to 21 (10101 binary) and EXTEN to 01 (Rising Edge)
//  // (21 << 5) sets the trigger source
//  // (1 << 10) sets the rising edge
//  ADC1->CFGR |= (21U << 5) | (1U << 10);
//
//  // Repeat for ADC2
//  ADC2->CFGR &= ~(0x3F << 5);
//  ADC2->CFGR |= (21U << 5) | (1U << 10);

  /*This defines the sensitivity. The ADC will trigger the exact moment
   * the HRTIM signal transitions from Low to High (the Rising Edge).*/


  // -------- ENABLE ADC --------
  // Enable ADC1 and ADC2 - This powers up the internal analog circuitry.
  /* This bit is set by software to enable the ADC. The ADC is effectively ready to operate once the flag ADRDY in ADC_ISR reg has been set.
     It is cleared by hardware when the ADC is disabled, after the execution of the ADDIS command. */
  ADC1->CR |= (1U<<0);
  ADC2->CR |= (1U<<0);

  // Wait until ADRDY
  /*This bit is set by hardware after the ADC has been enabled (bit ADEN = 1) and when the ADC reaches a state where it
    is ready to accept conversion requests. It is cleared by software writing 1 to it. 1: ADC is ready to start conversion
  */
  while ((ADC1->ISR & (1U<<0)) == 0) {}
  while ((ADC2->ISR & (1U<<0)) == 0) {}

  // Clear the ADRDY properly (write 1 to clear) - Cleanup
  ADC1->ISR |= (1U<<0);
  ADC2->ISR |= (1U<<0);


}

//void DAC_CONFIG(void)
//{
//	// Clock config
//	RCC->APB2ENR |= (1U << 0);   // SYSCFG for COMP
//	RCC->AHB2ENR |= (1U << 16); // DAC1
//
//	/* Vref = 1.65 + I_peak × 0.8
//	   DAC = (Vref / 3.3) × 4095
//
//	   1A peak means
//	   Vref = 1.65 + 1 × 0.8 = 2.45 V
//	   DAC = (2.45 / 3.3) × 4095 ≈ 3040
//	   DAC1->DHR12R1 = 3040;
//
//	 */
//
//	// Trigger not needed (software trigger)
//	DAC1->SWTRIGR |= (1U << 0);
//
//	// Enable DAC channel 1
//	DAC1->CR |= (1U << 0);
//
//	// Set output value (12-bit right aligned)
//	DAC1->DHR12R1 = 2100;  // ~1.65V reference

//}
//
//void GPIO_PA4_DAC_OUT(void)
//{
//
////	// PA6 → COMP1_OUT
////	GPIOA->MODER &= ~(1U<<13);
////	GPIOA->MODER |=  (1U<<12);
////
////	// AF8 = COMP1_OUT
////	GPIOA->AFR[0] &= ~(0xF << 24);
////	GPIOA->AFR[0] |=  (8U << 24);
//
//    // PA4 → analog mode (11)
//    GPIOA->MODER |= (1U << 8);
//    GPIOA->MODER |= (1U << 9);
//
//}
//
//void GPIO_PA6_COMP_OUT(void)
//{
//
//	// PA6 → COMP1_OUT
//	GPIOA->MODER |=  (1U<<13);
//	GPIOA->MODER &= ~(1U<<12);
//
//	// AF8 = COMP1_OUT
//	GPIOA->AFR[0] &= ~(0xF << 24);
//	GPIOA->AFR[0] |=  (8U << 24);
//
//}
//
//void COMP_CONFIG(void)
//{
//	//+ve input selection
//	//COMP1->CSR &= ~(1U << 8); // default PA1
//
//	//-ve input selection
//	COMP1->CSR |= (1U << 6);//DAC1_CH1 (101)
//	COMP1->CSR &= ~(1U << 5);
//	COMP1->CSR |= (1U << 4);
//
//	//Polarity
//	//non-inverted: output = 1 when INP > INM
//	COMP1->CSR &= ~(1U << 15);
//
//	// Some hysteresis (recommended)
//	// Blanking?
//
//	//Comparator 1 enable
//	COMP1->CSR |= (1U << 0);
//}


//-------- DMA1 CONFIG --------
void DMA_INIT (void)
{
	// Enable DMA1, DMAMUX1 clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMAMUX1EN;

	// ADC1 is usually DMAMUX Request 5, ADC2 is Request 36
	DMAMUX1_Channel0->CCR = 5U; // Map ADC1 to DMA1 Channel 1 (DMAMUX Channel 0 always connects to DMA1 Channel 1
	//DMAMUX Channel 1 always connects to DMA1 Channel 2)
	DMAMUX1_Channel1->CCR = 36U; // Map ADC2 to DMA1 Channel 2

	DMA1_Channel1->CCR = 0;   // RESET EVERYTHING FIRST
	DMA1_Channel2->CCR = 0;   // RESET EVERYTHING FIRST

	// --- DMA1 Channel 1 (For ADC1) ---
	DMA1_Channel1->CPAR = (uint32_t)&(ADC1->DR);   // Peripheral address
	DMA1_Channel1->CMAR = (uint32_t)ADC1_Data;      // Memory address
	DMA1_Channel1->CNDTR = 1;                       // 1 conversions in sequence

	// Memory increment
	DMA1_Channel1->CCR |= (1U << 7);

	// Peripheral size to half word - 16bits
	DMA1_Channel1->CCR &= ~(1U<<9);
	DMA1_Channel1->CCR |= (1U << 8);

	// Memory size to half word - 16bits
	DMA1_Channel1->CCR &= ~(1U<<11);
	DMA1_Channel1->CCR |= (1U << 10);

	// Circular mode enable
	/*It tells the DMA controller to reset its "Destination Pointer"
	  back to the start of your array once it reaches the end.*/
	DMA1_Channel1->CCR |= (1U << 5);

	// Channel Enable
	DMA1_Channel1->CCR |= (1U << 0);

	// --- DMA1 Channel 2 (For ADC2) ---
	DMA1_Channel2->CPAR = (uint32_t)&(ADC2->DR); // Peripheral address
	DMA1_Channel2->CMAR = (uint32_t)ADC2_Data; // Memory address
	DMA1_Channel2->CNDTR = 1; // 1 conversions in sequence

	// Memory increment
	DMA1_Channel2->CCR |= (1U << 7);

	// Peripheral size to half word - 16bits
	DMA1_Channel2->CCR &= ~(1U<<9);
	DMA1_Channel2->CCR |= (1U << 8);

	// Memory size to half word - 16bits
	DMA1_Channel2->CCR &= ~(1U<<11);
	DMA1_Channel2->CCR |= (1U << 10);

	// Circular mode enable
	DMA1_Channel2->CCR |= (1U << 5);

	// Channel Enable
	DMA1_Channel2->CCR |= (1U << 0);

}

//-------- HBRIDGE GPIO INIT --------
void HBRIDGE_GPIO_INIT(void)
{
    // Enable GPIOA clock
    RCC->AHB2ENR |= (1U<<0); // GPIOA

    //PA10 (M1), PA11 (M2) as OUTPUT (01)
    GPIOA->MODER &= ~(1U<<21); //PA10
    GPIOA->MODER |= (1U<<20);

    GPIOA->MODER &= ~(1U<<23); //PA11
    GPIOA->MODER |= (1U<<22);

    // Push-pull
    GPIOA->OTYPER &= ~(1U<<10); //PA10
    GPIOA->OTYPER &= ~(1U<<11); //PA11

    // High speed (10) (change to very high speed???)
    GPIOA->OSPEEDR |= (1U<<21); //PA10
    GPIOA->OSPEEDR &= ~(1U<<20);

    GPIOA->OSPEEDR |= (1U<<23); //PA11
    GPIOA->OSPEEDR &= ~(1U<<22);

    //--- QUADRANT INDICATION LED INIT ---

    // Enable GPIOB clock
    RCC->AHB2ENR |= (1U<<1);

    // PB7 as output
    GPIOB->MODER &= ~(1U<<15);
    GPIOB->MODER |=  (1U<<14);

    // Push-pull
    GPIOB->OTYPER &= ~(1U << 7);

    // LED OFF initially (active LOW)
    GPIOB->BSRR = (1U << 7);

}

// ----- H-BRIDGE STATES -----

// ----- H-BRIDGE OFF -----
void HBRIDGE_OFF(void)
{
    GPIOA->BSRR = (1U << (10 + 16)); // M1 LOW (PA10)
    GPIOA->BSRR = (1U << (11 + 16)); // M2 LOW (PA11)

    //--- LED ---
    GPIOB->BSRR = (1U << 7); // HIGH -> OFF
}

// ----- H-BRIDGE +VE -----
void HBRIDGE_POSITIVE(void)
{
    GPIOA->BSRR = (1U << 10);        // M1 HIGH (PA10)
    GPIOA->BSRR = (1U << (11 + 16)); // M2 LOW (PA11)

    //--- LED ---
    GPIOB->BSRR = (1U << (7 + 16)); // LOW → ON
}

// ----- H-BRIDGE -VE -----
void HBRIDGE_NEGATIVE(void)
{
    GPIOA->BSRR = (1U << (10 + 16)); // M1 LOW (PA10)
    GPIOA->BSRR = (1U << 11);        // M2 HIGH (PA11)

    //--- LED ---
    GPIOB->BSRR = (1U << (7 + 16)); // LOW → ON

//    // Blink LED 10 times
//    for (int i = 0; i < 2; i++)
//    {
//        // LED ON (active LOW)
//        GPIOB->BSRR = (1U << (7 + 16));
//        HAL_Delay(250);
//
//        // LED OFF
//        GPIOB->BSRR = (1U << 7);
//        HAL_Delay(250);
//    }
//
//    // Final state → LED ON (stable)
//    GPIOB->BSRR = (1U << (7 + 16));
}
