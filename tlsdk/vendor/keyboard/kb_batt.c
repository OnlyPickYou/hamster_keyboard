/*
 * kb_batt.c
 *
 *  Created on: Feb 14, 2014
 *      Author: xuzhen
 */
#include "../../proj/tl_common.h"
#include "kb.h"
#include "../../proj_lib/pm.h"
#include "../../proj_lib/pm_8366.h"
#include "kb_custom.h"
#include "kb_batt.h"
#include "kb_led.h"

#ifndef DBG_BATT_LOW
#define DBG_BATT_LOW    0
#endif

void kb_batt_det_process( kb_status_t *kb_status )
{
	u8 batt_chn = (kb_status->vbat_channel == 0xFF) ? COMP_ANA9 : kb_status->vbat_channel;    //COMP_GP5
	u8 vbat_chn = MOUSE_BATT_CHN_REAL_MASK & batt_chn;
	if ( DBG_BATT_LOW || battery_low_detect_auto(vbat_chn) ){
		//battery low, led alarm
		kb_device_led_setup( kb_led_cfg[KB_LED_BAT_LOW] );
	}
}

