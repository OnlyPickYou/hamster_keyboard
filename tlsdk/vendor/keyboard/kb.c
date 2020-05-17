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

void mouse_sync_process( kb_status_t *kb_status){

#if 0
    static u16 pairing_time = 0;
    u32 pairing_end = 0;
#if MOUSE_GOLDEN_VPATTERN
    if ( HOST_NO_LINK ){
        kb_status->mouse_mode = STATE_POWERON;
    }
#endif
    static u8 led_linked_host = 1;      //mouse, wakeup from deep-sleep, can blinky 3 times when linked with dongle
    if ( kb_status->mouse_mode == STATE_POWERON ) {        
        //debug mouse get access code-1 from flash/otp
        if ( GET_HOST_ACCESS_CODE_FLASH_OTP != U16_MAX ){
		    rf_set_access_code1 ( rf_access_code_16to32( GET_HOST_ACCESS_CODE_FLASH_OTP ) );            
        }
        else{            
            rf_set_access_code1 ( U32_MAX );            //powe-on pkt link fail
        }

        mouse_sync_status_update( kb_status );
	}
#if DEVICE_LED_MODULE_EN
    else{
        //if linked with dongle, led blinky 3 times
        if( led_linked_host && (kb_status->no_ack == 0) ){
            mouse_led_setup( mouse_led_cfg[E_LED_RSVD] );
            led_linked_host = 0;
        }
    }
#endif
    if( kb_status->mouse_mode == STATE_PAIRING ){
    	if( kb_status->no_ack == 0 ){
    	    pairing_end = U8_MAX;        //pairing OK
    	}
        if( pairing_time >= 1024 ){
            pairing_end = 1;             //pairing fail, time out
        }
        else{            
            pairing_time += 1;
        }
        
        if( pairing_end ){
            pairing_time = 0;           //pairing-any-time could re-try many time
            mouse_led_setup( mouse_led_pairing_end_cfg_cust(pairing_end) );
            mouse_sync_status_update( kb_status );
        }
        else{
            mouse_led_setup( mouse_led_cfg[E_LED_PAIRING] );
        }
    }
    else if ( kb_status->mouse_mode != STATE_EMI ){
        mouse_sync_status_update( kb_status );
    }
#endif
}
//u32 debug_last_wakeup_level;

#define   DEBUG_NO_SUSPEND      0
static inline void mouse_power_saving_process( kb_status_t *kb_status ){

#if 0
    log_event( TR_T_MS_MACHINE);
#if MOUSE_DEEPSLEEP_EN
    device_sleep.quick_sleep = HOST_NO_LINK && QUICK_SLEEP_EN;

    if ( kb_status->mouse_mode <= STATE_PAIRING ){
        if ( kb_status->loop_cnt < 4096 )       //sync dongle time 4096 *8 ms most 
            device_sleep.quick_sleep = 0;        
    }
#endif
    device_sleep.device_busy = DEVICE_PKT_ASK || LED_EVENT_BUSY;
    if ( DEBUG_NO_SUSPEND || (kb_status->dbg_mode & STATE_TEST_0_BIT) ){	    
        device_sleep.mode = M_SUSPEND_0;
    }
    else{
        if ( device_sleep.mode == M_SUSPEND_0 ){
#if(!MOUSE_GOLDEN_VPATTERN)					//use as golden
            while(1);     //watch dog reset
#endif
        }
        device_sleep_mode_machine( &device_sleep );
    }

	if ( M_SUSPEND_MCU_SLP & device_sleep.mode ){
    	device_info_save(kb_status, 1);    	        //Save related information to 3.3V analog register
	}

#if(CHIP_8366_A1)
    device_sleep.wakeup_src = PM_WAKEUP_TIMER;  //A1 wheel no need wkup 8ms suspend
#else
	device_sleep.wakeup_src = PM_WAKEUP_CORE | PM_WAKEUP_TIMER;  //wheel and timer can wakeup 8_ms sleep
#endif
	//Suspend, deep sleep GPIO, 32K setting
	if ( device_sleep.mode & M_SUSPEND_100MS ){
#if	MOUSE_SENSOR_MOTION
        device_sleep.wakeup_time = sensor_motion_detct ? 200 : 100;
#else
        device_sleep.wakeup_time = 100;
#endif
        device_sleep.wakeup_src = PM_WAKEUP_CORE | PM_WAKEUP_PAD | PM_WAKEUP_TIMER;
        device_sync = 0;        //ll_channel_alternate_mode ();
	}

#if MOUSE_DEEPSLEEP_EN
	else if ( M_SUSPEND_MCU_SLP & device_sleep.mode ){
		device_sleep.wakeup_time = 0;		
		device_sleep.wakeup_src = PM_WAKEUP_PAD;
        device_sync = 0;        //ll_channel_alternate_mode ();

	}
#endif
    log_event( TR_T_MS_SLEEP );
#endif
}

//low-end mouse higher current, active for longer time
#if MOUSE_SW_CUS
static inline void mouse_power_consume_process (void){
    if ( M_SUSPEND_8MS & device_sleep.mode ){
        WaitUs(mouse_cust_low_end_delay_time);
    }
}
#endif

void mouse_task_when_rf ( void ){
#if 0
    dbg_led_high;
	//clear mouse_event_data	
	*((u32 *)(kb_status.data)) = 0;


    mouse_led_process(kb_status.led_define);
    mouse_rf_post_proc(&kb_status);
    mouse_sync_process(&kb_status);

    log_event( TR_T_MS_BTN );
    mouse_power_saving_process(&kb_status);

#if    MOSUE_BATTERY_LOW_DETECT
    static u16 batt_det_count = 0;
    if( (kb_status.rf_mode != RF_MODE_IDLE) && (device_sleep.mode == M_SUSPEND_8MS) ){
        if ( ++batt_det_count >= mouse_batt_detect_time ){
            mouse_batt_det_process(&kb_status);
            batt_det_count = 0;
        }
    }
    else{
        batt_det_count = mouse_batt_detect_time;
    }
#endif

    kb_status.loop_cnt ++;
    dbg_led_low;
#endif
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


#if 0
void main_loop(void)
{
	if( MOUSE_EMI_4_FCC || (kb_status.mouse_mode == STATE_EMI) \
        || (kb_status.dbg_mode & STATE_TEST_EMI_BIT) ){
		mouse_emi_process(&kb_status);
	}
	else{
        log_event( TR_T_MS_EVENT_START );
#if MOUSE_SW_CUS
        if ( kb_status.high_end == MS_LOW_END )
            mouse_power_consume_process ();
#endif
#if (!MOUSE_NO_HW_SIM)       
        mouse_button_pull(&kb_status, MOUSE_BTN_LOW);
        mouse_sensor_data( &kb_status );
        mouse_button_detect(&kb_status, MOUSE_BTN_HIGH);        
        mouse_button_pull(&kb_status, MOUSE_BTN_HIGH);
#endif
        mouse_task_in_ram();
	}
}
#else

void main_loop(void)
{
    cpu_rc_tracking_en (RC_TRACKING_32K_ENABLE);
	key_scaned = kb_scan_key (kb_status.host_keyboard_status | kb_is_lock_pressed, !km_dat_sending);

	if(kb_status.kb_mode == STATE_EMI){
		kb_emi_process();
	}else{
		if(kb_status.kb_mode <= STATE_PAIRING){
			kb_paring_and_syncing_proc();
		}

		kb_rf_proc(key_scaned);
		kb_device_led_process();
		kb_batt_det_process(&kb_status);
		kb_pm_proc();
	}
	cpu_rc_tracking_disable;
	kb_status.loop_cnt++;

}
#endif
