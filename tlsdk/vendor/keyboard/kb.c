#include "../../proj/tl_common.h"
#include "../../proj/mcu/watchdog_i.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../common/rf_frame.h"
#include "../common/emi.h"
#include "../common/device_power.h"
#include "../../proj/drivers/keyboard.h"
#include "../link_layer/rf_ll.h"

#include "kb_info.h"
#include "kb.h"
#include "kb_batt.h"
#include "kb_rf.h"
#include "kb_led.h"
#include "kb_custom.h"
#include "kb_emi.h"
#include "trace.h"


#define led_cnt_rate    8

kb_status_t kb_status;
u32 key_scaned;
extern kb_data_t	kb_event;
extern int kb_is_lock_pressed;
void kb_sleep_wakeup_init(){
#ifdef reg_gpio_wakeup_en    
	reg_gpio_wakeup_en |= FLD_GPIO_WAKEUP_EN;	//gpio wakeup initial
#endif	
	reg_wakeup_en = FLD_WAKEUP_SRC_GPIO;        //core wakeup gpio enbable

}


kb_status_t* kb_proc_get_kb_status(void)
{
    return &kb_status;
}


void kb_platform_init( kb_status_t  *kb_status )
{
	kb_device_led_init(kb_status->led_gpio_lvd, kb_status->led_level_lvd, 5);

#if 0
	//Note:gpio_num defined as SWS, not enbale now
	if(kb_status.led_gpio_num){
		gpio_set_input_en( kb_status.led_gpio_num, 0 );
	    gpio_set_output_en( kb_status.led_gpio_num, 1 );
	    gpio_write(kb_status.led_gpio_num,0^kb_status.led_level_num);
	}
#endif

	//emi paring±ØÐëÔÚSTATE_POWERON?
	if(kb_status->kb_mode == STATE_POWERON){
		if( (kb_event.keycode[0] == VK_MINUS) && (kb_event.cnt == 1)){
			kb_status->kb_mode  = STATE_EMI;
		}else{
			kb_device_led_setup( kb_led_cfg[KB_LED_POWER_ON] );
			kb_status->kb_mode = STATE_SYNCING;
			memset(&kb_event,0,sizeof(kb_event));  //clear the numlock key when power on
		}
	}
}

void  user_init(void)
{
	swire2usb_init();

	kb_status.no_ack = 1;

	kb_custom_init(&kb_status);
	kb_info_load();

	int status = kb_status.host_keyboard_status | (kb_status.kb_mode == STATE_POWERON ? KB_NUMLOCK_STATUS_POWERON : 0);
	kb_scan_key (status, 1);
	kb_status.host_keyboard_status = KB_NUMLOCK_STATUS_INVALID;

    if ( kb_status.kb_mode == STATE_POWERON )
    	kb_device_led_setup(kb_led_cfg[KB_LED_POWER_ON]);

	kb_platform_init(&kb_status);
	kb_pm_init();
    kb_rf_init();
    rf_set_power_level_index (kb_cust_tx_power_paring);

    gpio_pullup_dpdm_internal( FLD_GPIO_DM_PULLUP | FLD_GPIO_DP_PULLUP );
}

_attribute_ram_code_ void irq_handler(void)
{

	u32 src = reg_irq_src;
#if(!CHIP_8366_A1)
	if(src & FLD_IRQ_GPIO_RISC2_EN){
		gpio_user_irq_handler();
		reg_irq_src = FLD_IRQ_GPIO_RISC2_EN;
	}
#endif
	u16  src_rf = reg_rf_irq_status;
	if(src_rf & FLD_RF_IRQ_RX){
		irq_device_rx();
		//printf("It is rx interrupt\n");
	}

	if(src_rf & FLD_RF_IRQ_TX){
		irq_device_tx();
	}
}

_attribute_ram_code_ void mouse_task_in_ram( void ){

#if 0
    p_task_when_rf = mouse_task_when_rf;
    cpu_rc_tracking_en (RC_TRACKING_32K_ENABLE);
    mouse_rf_process(&kb_status);
    cpu_rc_tracking_disable;
    if (p_task_when_rf != NULL) {
       (*p_task_when_rf) ();
    }
    dbg_led_high;
#if MOUSE_SW_CUS
	if ( kb_status.high_end == MS_HIGHEND_250_REPORTRATE )
	    ll_add_clock_time (4000);
    else        
        ll_add_clock_time (8000);
#else
	ll_add_clock_time (8000);
#endif

    device_sleep_wakeup( &device_sleep );
    dbg_led_low;
#endif
}


void main_loop(void)
{
    cpu_rc_tracking_en (RC_TRACKING_32K_ENABLE);
	key_scaned = kb_scan_key (kb_status.host_keyboard_status, !km_dat_sending);

	if(kb_status.kb_mode == STATE_EMI){
		kb_emi_process();
	}else{
		if(kb_status.kb_mode <= STATE_PAIRING){
			kb_paring_and_syncing_proc();
		}

		kb_rf_proc(key_scaned);
		kb_device_led_process();
	    if (p_task_when_rf != NULL) {
	       (*p_task_when_rf) ();
	    }
		kb_pm_proc();
	}
	cpu_rc_tracking_disable;
	kb_status.loop_cnt++;

}
