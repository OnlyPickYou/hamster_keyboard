/*
 * kb_batt.c
 *
 *  Created on: Sep 1, 2014
 *      Author: qcmao
 */

#include "../../proj/tl_common.h"
#include "../../proj_lib/pm.h"
#include "kb_custom.h"
#include "kb_batt.h"
#include "../../proj/drivers/adc.h"

int kb_batt_det_flg = 0;
void kb_batt_det_init(void)
{
	gpio_set_input_en(GPIO_BAT_DET,0);
	reg_adc_ctrl = 0x00;  //no auto
	adc_set_chn0_sampling(ADC_SAMPLING_SYCLE_6,ADC_SAMPLING_RES_14BIT);
	reg_adc_mod = MASK_VAL(FLD_ADC_MOD, CLK_FHS_MZ, FLD_ADC_CLK_EN, 1);
	reg_adc_step_l = 4;
	//reg_adc_mod_h |= FLD_ADC_MOD_H_CLK;
	reg_adc_chn_m_sel =  ADC_CHN_BAT_DET;//sel
	reg_adc_ref = FLD_ADC_VREF_1P3V;
}

int kb_batt_det_process(void)
{
	int adc_result_mv;

	reg_adc_manu_start = FLD_ADC_MANU_START;
	sleep_us(5);
	adc_result_mv = ( (reg_adc_dat_byp_outp & 0x3fff) * 1300 ) >>14;
	if ( adc_result_mv < 1200 ){
		kb_led_setup( kb_led_cfg[E_LED_BAT_LOW] );
	}
	return adc_result_mv;
}
