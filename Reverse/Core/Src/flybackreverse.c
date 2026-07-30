/*
	 PIN MAP

	 -------------------------------------------------------------
	 PA3 - ADC1_IN4 - secondary current - ADC1_data[0] - Pin 17 - COMP2_INP
	 PA2 - ADC1_IN3 - secondary voltage - ADC1_data[1] - Pin 14

	 PA0 - ADC2_IN1 - primary voltage - ADC2_data[0] - Pin 12

	 PA1 - ADC2_IN2 - primary current - ADC2_data[1] - Pin 13 - COMP1_INP



	 -------------------------------------------------------------

	 PWMA - HRTIM_CHA1 - PA8 - Pin 42
	 PWMB - HRTIM_CHA2 - PA9 - Pin 43

	 -------------------------------------------------------------

	 M1 Signal - PA10 - Pin 44
	 M2 Signal - PA11 - Pin 45

	 -------------------------------------------------------------

*/

#include "main.h"

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
	  HRTIM1_TIMA->CMP1xR = 1500U;
  }


  // -------- CLEAR OUTPUT CONFIG --------
  HRTIM1_TIMA->SETx1R = 0;
  HRTIM1_TIMA->RSTx1R = 0;

  HRTIM1_TIMA->SETx2R = 0;
  HRTIM1_TIMA->RSTx2R = 0;


  // -------- CH2 PWM CONFIG --------
  // CH2 HIGH at beginning of PWM period
  HRTIM1_TIMA->SETx2R |= (1U << 2);   // PER event

  // CH2 LOW at CMP1 match
  HRTIM1_TIMA->RSTx2R |= (1U << 3);   // CMP1 event


  // -------- DISABLE DEADTIME --------
  HRTIM1_TIMA->OUTxR &= ~(1U << 8);


  // -------- OUTPUT ENABLE CONFIG --------
  // Disable CH1 output
  HRTIM1_COMMON->ODISR |= (1U << 0);

  // Enable CH2 output
  HRTIM1_COMMON->OENR |= (1U << 1);



  //Setting output 1 high when the counter=CMP1
  //HRTIM1_TIMA->SETx1R |= (1U<<3);

  //Resetting output 1 low at the beginning of PWM cycle (PER)
  //HRTIM1_TIMA->RSTx1R |= (1U<<2);

  //Setting and resetting output 1 is automatic with dead time enable

  	  	  	  //----- HRTIM INTERRUPT CONFIG -----
  	  	  	  /* We are now interrupting the processor by this statement, if interrupt the processor and dont put an interrupt service routine
   	   	   	   then it will go into an infinite wait loop. So we want to clear the flag.*/

  	  	  	  //Interrupt at period
  	  	  	  HRTIM1_TIMA->CMP2xR = HRTIM1_TIMA->CMP1xR * 50U/100U; //Set CMP2 to match the Period
  	  	  	  HRTIM1_TIMA->TIMxDIER |= (1U<<1); //Enable the CMP2 Interrupt i.e Period

  	  	  	  //NVIC
  	  	  	  HAL_NVIC_SetPriority(HRTIM1_TIMA_IRQn, 0, 0);
  	  	  	  HAL_NVIC_EnableIRQ(HRTIM1_TIMA_IRQn);

  //Enable dead-time
  //HRTIM1_TIMA->OUTxR |= (1U<<8);

  //Specifying dead-time and dead time pre-scaler (170MHz)
  //PriMOS tr=10ns, td(on)=10ns; tf=8ns, td(off)=22ns =>DTF = tf+td(off)-td(on,secMOS) = 30ns-14ns=16ns=>35ns
  //SecMOS tr=16ns, td(on)=14ns; tf=8ns, td(off)=33ns =>DTR = tf+td(off)-td(on,priMOS) = 41ns-10ns=31ns=>50ns
  //We need 35ns and 50ns of dead time; so if we use tDTG of 2.94ns (010) we get 12 and 17 as values
  /* Timer and the Dead-time Unit are two different "rooms" inside the same building. They don't have to use the same clock speed.
   * t_DTG stands for Dead-time Generator clock. If t_DTG = 2.94ns, then every 1 you write in the register adds 2.94ns of delay.*/
  //HRTIM1_TIMA->DTxR =  (2U << 10) | (12U << 16) | (17U << 0);

  //Enable timer A1 and A2
  /*It basically "connects" the internal PWM signal to the physical GPIO pin.*/
  //HRTIM1_COMMON->OENR |= (1U<<0);
  //HRTIM1_COMMON->OENR |= (1U<<1);

//  	  	  //----- HRTIM TRIGGER FOR ADC CONFIG -----
//
//  	  	  //ADC start of conversion trigger (hardware trigger) - Timer A
//  	  	  //ADC trigger 1 update source is Timer A (These bits define the source which triggers the update
// 	  	  //of the HRTIM_ADC1R register (transfer from pre-load to active register))
//  	  	  HRTIM1_COMMON->CR1 &= ~(1U<<18);
//  	  	  HRTIM1_COMMON->CR1 &= ~(1U<<17);
//  	  	  HRTIM1_COMMON->CR1 |= (1U<<16);
//
//  	  	  //Select which event- Timer A reset (PWM end of cycle) will trigger ADC start of conversion
//  	  	  //HRTIM1_COMMON->ADC1R |= (1U<<14); //ADC trigger 1 on timer A reset and counter roll-over
//	      //HRTIM1_COMMON->ADC1R |= (1U<<13); //ADC trigger 1 on timer A period
//  	  	  // Place trigger at ~80% of ON-time
//  	  	  HRTIM1_TIMA->CMP3xR = (HRTIM1_TIMA->CMP1xR * 60U) / 100U;
//  	  	  // Enable ADC trigger 1 on timer A compare 3
//  	  	  HRTIM1_COMMON->ADC1R |= (1U<<11);
//	      /*ONE trigger source (TRG1)
//	    	BUT triggered at TWO moments:
//	        - RESET
//	        - PERIOD*/


  //----- GPIO CONFIG -----
  //Configuring PA8 and PA9 as HRTIM CHA1 and CHA2

  //Enable GPIOA
  RCC->AHB2ENR |= (1U<<0);

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
  //ADC sequence length: 2 (2 for ADC1 and 1 for ADC2)
  //Once we trigger conversion how many channels should it convert.
  ADC1->SQR1 &= ~(1U<<3);
  ADC1->SQR1 &= ~(1U<<2);
  ADC1->SQR1 &= ~(1U<<1);
  ADC1->SQR1 |= (1U<<0);

  ADC2->SQR1 &= ~(1U<<3);
  ADC2->SQR1 &= ~(1U<<2);
  ADC2->SQR1 &= ~(1U<<1);
  ADC2->SQR1 &= ~(1U<<0);

  //ADC conversion sequence order
  //Write the channel number(not ADC number) explicitly

  //First priority (SQ1)
  //ADC1_IN4 => Secondary current
  ADC1->SQR1 &= ~(1U<<10);
  ADC1->SQR1 &= ~(1U<<19);
  ADC1->SQR1 |= (1U<<8);
  ADC1->SQR1 &= ~(1U<<7);
  ADC1->SQR1 &= ~(1U<<6);

  //ADC2_IN1 => Primary voltage
  ADC2->SQR1 &= ~(1U<<10);
  ADC2->SQR1 &= ~(1U<<9);
  ADC2->SQR1 &= ~(1U<<8);
  ADC2->SQR1 &= ~(1U<<7);
  ADC2->SQR1 |= (1U<<6);

  //Second priority (SQ2)
  //ADC1_IN3 => Secondary voltage
  ADC1->SQR1 &= ~(1U<<16);
  ADC1->SQR1 &= ~(1U<<15);
  ADC1->SQR1 &= ~(1U<<14);
  ADC1->SQR1 |= (1U<<13);
  ADC1->SQR1 |= (1U<<12);

  //  //ADC2_IN2 => Primary current
  //  ADC2->SQR1 &= ~(1U<<16);
  //  ADC2->SQR1 &= ~(1U<<15);
  //  ADC2->SQR1 &= ~(1U<<14);
  //  ADC2->SQR1 |= (1U<<13);
  //  ADC2->SQR1 &= ~(1U<<12);

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

//  ADC2->SMPR1 |= (1U<<8); //SMP2[2:0] for primary current in ADC2_IN2
//  ADC2->SMPR1 &= ~(1U<<7);
//  ADC2->SMPR1 |= (1U<<6);
//
//  ADC1->SMPR1 |= (1U<<11); //SMP3[2:0] for secondary voltage in ADC1_IN3
//  ADC1->SMPR1 &= ~(1U<<10);
//  ADC1->SMPR1 |= (1U<<9);

  ADC1->SMPR1 &= ~(1U<<14); //SMP4[2:0] for secondary current in ADC1_IN4
  ADC1->SMPR1 &= ~(1U<<13);
  ADC1->SMPR1 &= ~(1U<<12);

  ADC1->SMPR1 &= ~(1U<<11); //SMP3[2:0] for secondary voltage in ADC1_IN3
  ADC1->SMPR1 &= ~(1U<<10);
  ADC1->SMPR1 &= ~(1U<<9);

  ADC2->SMPR1 &= ~(1U<<5); //SMP1[2:0] for primary voltage in ADC2_IN1
  ADC2->SMPR1 &= ~(1U<<4);
  ADC2->SMPR1 &= ~(1U<<3);



  // -------- ADC CONFIG --------
  //Configure ADC1 and ADC2 for DMA1
  //DMA1 enable
  ADC1->CFGR |= (1U << 0);
  ADC2->CFGR |= (1U << 0);

  //DMA1 Circular Mode - It tells the ADC to keep generating DMA "requests" indefinitely.
  ADC1->CFGR |= (1U << 1);
  ADC2->CFGR |= (1U << 1);

  //Discontinuous mode disable
  ADC1->CFGR &= ~(1U<<16);
  ADC2->CFGR &= ~(1U<<16);

  //  // --- Continuous mode ---
  ADC1->CFGR |= (1U << 13);
  ADC2->CFGR |= (1U << 13);

  ADC1->CFGR |= (1U<<12); // OVRMOD
  ADC2->CFGR |= (1U<<12);

//  // --- External trigger enable and polarity selection for regular channels ---
//  ADC1->CFGR &= ~(1U << 11); // EXTEN = 01 (Hardware trigger detection on the rising edge)
//  ADC1->CFGR |= (1U << 10);
//
//  ADC2->CFGR &= ~(1U << 11); // EXTEN = 01 (Hardware trigger detection on the rising edge)
//  ADC2->CFGR |= (1U << 10);
//
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



//-------- DMA1 CONFIG --------
void DMA_INIT (void)
{
	// Enable DMA1, DMAMUX1 clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMAMUX1EN;

	// ADC1 is DMAMUX Request 5, ADC2 is Request 36
	DMAMUX1_Channel0->CCR = 5U; // Map ADC1 to DMA1 Channel 1 (DMAMUX Channel 0 always connects to DMA1 Channel 1
	//DMAMUX Channel 1 always connects to DMA1 Channel 2)
	DMAMUX1_Channel1->CCR = 36U; // Map ADC2 to DMA1 Channel 2

	DMA1_Channel1->CCR = 0;   // RESET EVERYTHING FIRST
	DMA1_Channel2->CCR = 0;   // RESET EVERYTHING FIRST

	// --- DMA1 Channel 1 (For ADC1) ---
	DMA1_Channel1->CPAR = (uint32_t)&(ADC1->DR);   // Peripheral address
	DMA1_Channel1->CMAR = (uint32_t)ADC1_Data;      // Memory address
	DMA1_Channel1->CNDTR = 2;                       // 2 conversions in sequence

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

void GPIO_SINK_INIT(void)
{
	// Enable GPIOB clock
	RCC->AHB2ENR |= (1U<<1);

	// Set PB5 as output (MODER = 01)
	GPIOB->MODER &= ~(1U << 11);
	GPIOB->MODER |=  (1U << 10);

	// Push-pull
	GPIOB->OTYPER &= ~(1U << 5);

	// High speed
	GPIOB->OSPEEDR |= (1U<<11);
	GPIOB->OSPEEDR &= ~(1U<<10);

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

    // PB7 as output
    GPIOB->MODER &= ~(1U<<13);
    GPIOB->MODER |=  (1U<<12);

    // Push-pull
    GPIOB->OTYPER &= ~(1U << 7);
    // Push-pull
    GPIOB->OTYPER &= ~(1U << 6);

    // LED OFF initially (active LOW)
    GPIOB->BSRR = (1U << 7);
    GPIOB->BSRR = (1U << 6);

}

// ----- H-BRIDGE STATES -----

// ----- H-BRIDGE OFF -----
void HBRIDGE_OFF(void)
{
    GPIOA->BSRR = (1U << (10 + 16)); // M1 LOW (PA10)
    GPIOA->BSRR = (1U << (11 + 16)); // M2 LOW (PA11)

    //--- LED ---
    GPIOB->BSRR = (1U << 6); // HIGH -> OFF
}

// ----- H-BRIDGE +VE -----
void HBRIDGE_POSITIVE(void)
{
    GPIOA->BSRR = (1U << 10);        // M1 HIGH (PA10)
    GPIOA->BSRR = (1U << (11 + 16)); // M2 LOW (PA11)

    //--- LED ---
    GPIOB->BSRR = (1U << (6 + 16)); // LOW → ON
}

// ----- H-BRIDGE -VE -----
void HBRIDGE_NEGATIVE(void)
{
    GPIOA->BSRR = (1U << (10 + 16)); // M1 LOW (PA10)
    GPIOA->BSRR = (1U << 11);        // M2 HIGH (PA11)

    //--- LED ---
    GPIOB->BSRR = (1U << (6 + 16)); // LOW → ON

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

