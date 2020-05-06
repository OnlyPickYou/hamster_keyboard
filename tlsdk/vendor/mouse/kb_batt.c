/*
 * mouse_batt.c
 *
 *  Created on: Feb 14, 2014
 *      Author: xuzhen
 */
#include "../../proj/tl_common.h"
#include "kb.h"
#include "../../proj_lib/pm.h"
#include "kb_custom.h"
#include "kb_batt.h"
#include "kb_led.h"

#ifndef DBG_BATT_LOW
#define DBG_BATT_LOW    0
#endif

#if MOSUE_BATTERY_LOW_DETECT

void kb_batt_det_process( kb_status_t *kb_status ){

#if 0
    #define batt_chn (kb_status->hw_define->vbat_channel)
    if (  batt_chn & MOUSE_BATT_CHN_REUSE_FLAG ){   
        u32 gpio_batt_btn = kb_status->hw_define->button[(batt_chn >> 5) & 3];
        gpio_setup_up_down_resistor( gpio_batt_btn, PM_PIN_UP_DOWN_FLOAT );
        WaitUs(50);
    }
    if ( !(batt_chn & MOUSE_BATT_CHN_REUSE_FLAG) || ((kb_status->data->btn & FLAG_BUTTON_MIDDLE) == 0) ){
        //battery low, led alarm
        u8 vbat_chn = MOUSE_BATT_CHN_REAL_MASK & batt_chn;
        if ( DBG_BATT_LOW || battery_low_detect_auto( vbat_chn == U8_MAX ? COMP_GP6 : vbat_chn ) ){
        	kb_device_led_setup( kb_led_cfg[KB_LED_BAT_LOW] );
        }
    }
    if ( batt_chn & MOUSE_BATT_CHN_REUSE_FLAG ){   
        u32 gpio_batt_btn = kb_status->hw_define->button[(batt_chn >> 5) & 3];
        gpio_setup_up_down_resistor( gpio_batt_btn, PM_PIN_PULLUP_1M );
    }
#endif
}


#endif
