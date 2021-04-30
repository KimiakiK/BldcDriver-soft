/*
 * drv_bldc.c
 *
 *  Created on: Sep 21, 2020
 *      Author: kimi
 */

/********** Include **********/

#include "stm32f3xx_hal.h"
#include "math.h"
#include "drv_bldc.h"

/********** Define **********/

#define DRV_BLDC_DEBUG

#define MAX (0.6f)

/********** Type **********/

/********** Constant **********/

/********** Variable **********/

static float radian;
static float input;

/********** Function Prototype **********/

static void drvBldcStartPwmOutput(void);

/********** Function **********/

void DrvBldcInit()
{
	radian = 0.0f;
	input = 1.0f;

	drvBldcStartPwmOutput();
}

void DrvBldcMain(uint16_t ad_value)
{
	input = (float)ad_value / 4096.0f;
}

void DrvBldcIntControl(void)
{
	/* カウンタがダウンカウンタの時に処理(割り込み2回おき) */
	if ((TIM1->CR1 & 0x0010) == 0x0010) {
#ifdef DRV_BLDC_DEBUG
		/* テストピン PB0 をトグル */
		GPIOB->ODR = ~GPIOB->ODR & 0x00000001;
#endif /* DRV_BLDC_DEBUG */

		radian += M_PI / (input * 500.0f + 1.0f);
		if (radian > M_PI * 2.0f) {
			radian -= M_PI * 2.0f;
		}
		TIM1->CCR1 = (cosf(radian) * MAX + 1.0f) * (float)(TIM1->ARR / 2);
		TIM1->CCR2 = (cosf(radian + (M_PI * 2.0f / 3.0f)) * MAX + 1.0f) * (float)(TIM1->ARR / 2);
		TIM1->CCR3 = (cosf(radian + (M_PI * 4.0f / 3.0f)) * MAX + 1.0f) * (float)(TIM1->ARR / 2);

	}
}

static void drvBldcStartPwmOutput(void)
{
#ifdef DRV_BLDC_DEBUG
	/* 更新割り込み有効化 */
	TIM1->DIER = TIM1->DIER | 0x0001;
#endif /* DRV_BLDC_DEBUG */

	/* チャネル出力許可 */
	TIM1->CCER = TIM1->CCER | 0x00000555;

	/* 出力許可 MOE=1 */
	TIM1->BDTR = TIM1->BDTR | 0x00008000;

	/* カウント開始 CEN=1 */
	TIM1->CR1 = TIM1->CR1 | 0x01;
}
