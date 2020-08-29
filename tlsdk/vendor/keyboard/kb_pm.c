/*
 * kb_pm.c
 *
 *  Created on: 2015-1-21
 *      Author: Administrator
 */



#include "../../proj/tl_common.h"
#include "../../proj_lib/pm.h"
#include "../../proj_lib/rf_drv.h"
#include "../common/rf_frame.h"

#include "kb_info.h"
#include "kb_led.h"
#include "kb_rf.h"
#include "kb_pm.h"
#include "kb.h"

extern u32	scan_pin_need;
extern int device_sync;

kb_slp_cfg_t kb_sleep = {
	SLEEP_MODE_BASIC_SUSPEND,  //mode
	0,                  //device_busy
	0,					//quick_sleep
	0,                  //wakeup_src

    0,
    166,  //12ms unit
    0,
    19,  //100 ms unit

    0,
    0,
};

//_attribute_ram_code_
inline void kb_slp_mode_machine(kb_slp_cfg_t *s_cfg)
{
    if ( s_cfg->device_busy ){
        s_cfg->mode = SLEEP_MODE_BASIC_SUSPEND;
        s_cfg->cnt_basic_suspend = 0;
        s_cfg->cnt_long_suspend = 0;
    }
    else if (s_cfg->mode == SLEEP_MODE_BASIC_SUSPEND){
        s_cfg->cnt_basic_suspend ++;
        if ( s_cfg->cnt_basic_suspend >= s_cfg->thresh_basic_suspend ){
            s_cfg->mode = SLEEP_MODE_LONG_SUSPEND;
            s_cfg->cnt_long_suspend = 0;
        }
    }


    if ( s_cfg->mode == SLEEP_MODE_LONG_SUSPEND ) {
		s_cfg->cnt_long_suspend ++;
		if ( s_cfg->cnt_long_suspend > s_cfg->thresh_long_suspend )	{
			s_cfg->mode = SLEEP_MODE_DEEPSLEEP;
		}
	}

    if ( s_cfg->quick_sleep ) {
        s_cfg->mode = SLEEP_MODE_DEEPSLEEP;
	}
}


void kb_pm_init(void)
{

#ifdef reg_gpio_wakeup_en
	reg_gpio_wakeup_en |= FLD_GPIO_WAKEUP_EN;	//gpio wakeup initial
#endif


	reg_wakeup_en = FLD_WAKEUP_SRC_GPIO;        //core wakeup gpio enbable
    for ( int i=0; i<6; i++ ){  //driver pin wakeup
        gpio_enable_wakeup_pin(drive_pins[i], 0, 1);
        cpu_set_gpio_wakeup (drive_pins[i], 0, 1);   //pad  wakeup deep   : low active
    }

	kb_sleep.wakeup_tick = clock_time();
}


_attribute_ram_code_ void kb_pm_proc(void)
{

	extern u32  cpu_wakup_last_tick;
    kb_status_t *kb_status = NULL;
    kb_status = kb_proc_get_kb_status();


	kb_sleep.device_busy = ( kb_status->rf_sending || KB_LED_BUSY );
	kb_sleep.quick_sleep = (kb_status->no_ack > 400);
	if ( kb_status->kb_mode <= STATE_PAIRING && kb_status->loop_cnt < KB_NO_QUICK_SLEEP_CNT){
		kb_sleep.quick_sleep = 0;
	}

	kb_slp_mode_machine( &kb_sleep );


	kb_sleep.wakeup_src = PM_WAKEUP_TIMER;
	kb_sleep.wakeup_time = KB_SLEEP_BASIC_WAKEUP_TIME;

	if ( kb_sleep.mode ==  SLEEP_MODE_LONG_SUSPEND){
    	kb_sleep.wakeup_src = PM_WAKEUP_TIMER | PM_WAKEUP_CORE;
    	kb_sleep.wakeup_time = KB_SLEEP_LONG_WAKUP_TIME;

	}
	else if ( kb_sleep.mode == SLEEP_MODE_DEEPSLEEP){
		if(scan_pin_need){  //有按键按着  wait_deep状态，按键释放后进入deep
			kb_status->kb_mode = STATE_WAIT_DEEP;
			kb_sleep.mode = SLEEP_MODE_WAIT_DEEP;
			kb_sleep.next_wakeup_tick = KB_SLEEP_LONG_WAKUP_TIME;
		}
		else{
			kb_info_save();
			kb_sleep.wakeup_src = PM_WAKEUP_PAD;
		}
	}

    while ( !clock_time_exceed (cpu_wakup_last_tick, 500) );

	int wakeup_status = cpu_sleep_wakeup_rc (kb_sleep.mode == SLEEP_MODE_DEEPSLEEP, kb_sleep.wakeup_src, kb_sleep.wakeup_time);   // 8ms wakeup
	if(!(wakeup_status & 2)){
		device_sync = 0;	//ll_channel_alternate_mode ();
	}

}


