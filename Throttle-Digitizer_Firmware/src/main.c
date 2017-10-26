/**
  ******************************************************************************
  * @file    main.c
  * @author  Mitchell Johnston
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

/* includes */
#include "stm32f0xx.h"
#include "stm32f0xx_adc.h"
#include "stm32f0_discovery.h"
#include "stm32f0xx_dma.h"
#include "stm32f0xx_tim.h"
#include "throttle_digitizer_init.h"


/* private variables */
volatile float throttle = 0;
volatile uint32_t dutyCount;
volatile uint16_t throttleConverted[1];

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t periodCount=0; // TIM1 base timer AAR register value
uint16_t pulseCount=0; //TIM1 CCR register value for toggling edge (duty)



int main(void)
{
	  adc_init(throttleConverted);
	  timer_config();

	  /* Compute the value to be set in ARR register to generate signal frequency at 10Khz */
	  periodCount = (SystemCoreClock / 10000 ) - 1;
	  /* Compute CCR1 value to generate a duty cycle at 50% for channel 1 */
	  pulseCount = (uint16_t) (((uint32_t) 5 * (periodCount - 1)) / 10); // load 0% duty to begin

	  /* TIM1 clock enable */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);

	  /* Time Base configuration */
	  TIM_TimeBaseStructure.TIM_Prescaler = 0;
	  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	  TIM_TimeBaseStructure.TIM_Period = periodCount;
	  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	  /* Channel 1 Configuration in PWM mode */
	  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	  TIM_OCInitStructure.TIM_Pulse = pulseCount; // load timer with 0% duty to begin
	  TIM_OC1Init(TIM1, &TIM_OCInitStructure);

	  /* TIM1 counter enable */
	  TIM_Cmd(TIM1, ENABLE);

	  /* TIM1 Main Output Enable */
	  TIM_CtrlPWMOutputs(TIM1, ENABLE);


	  while (1)
	  {
		  throttle = (float)(ADC_MAX/throttleConverted[0]); // map ADC value to duty cycle percentage
		  pulseCount = (uint32_t)(periodCount)/throttle; // determine count to trigger pulse edge in TIM1_CCR register

		  TIM_OCInitStructure.TIM_Pulse = (uint16_t) pulseCount; // update CCR register on update event (counter overflow flag)
		  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
		  TIM_Cmd(TIM1, ENABLE);

		  /* TIM1 Main Output Enable */
		  TIM_CtrlPWMOutputs(TIM1, ENABLE);

	  }
}
