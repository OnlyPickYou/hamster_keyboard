/*
 * kb_rf.h
 *
 *  Created on: Sep 2, 2014
 *      Author: qcmao
 */

#ifndef KB_RF_H_
#define KB_RF_H_
#include "../../proj_lib/rf_drv_8266.h"
#include "../common/rf_frame.h"
#include "kb_main.h"
#include "kb_custom.h"

#define RF_MODE_IDLE		0
#define RF_MODE_SYNC		1
#define RF_MODE_PAIRING		2
#define RF_MODE_DATA		4


//kb_status.loop_cnt relative
#define KB_MANUAL_PARING_MOST	   (8000/KB_MAIN_LOOP_TIME_MS)   //手动配对最大时间	8s
#define KB_RF_SYNC_PKT_TH_NUM 	   (2000/KB_MAIN_LOOP_TIME_MS)    //RF没有回应时进入quick_sleep时间    2s
#define KB_NO_QUICK_SLEEP_CNT	   (18000/KB_MAIN_LOOP_TIME_MS)  //LINK最大时间	 			       18 + 2 = 20 s
#define KB_PARING_POWER_ON_CNT     64    						 //上电自动配对包

#define	DEVICE_PKT_ASK	    (kb_status.rf_mode != RF_MODE_IDLE)
#define HOST_NO_LINK        (kb_status.no_ack >= KB_RF_SYNC_PKT_TH_NUM)

#define device_never_linked (rf_get_access_code1() == U32_MAX)


typedef struct{
	u8  paring_key_1;
	u8  paring_key_2;
	u8  paring_key_cnt;
	u8  pair_status;
	u8  scan_status;
	u8  paring_enable;
	u8  rsv[2];
}kb_pair_info_t;

//extern led_cfg_t kb_led_cfg[];
extern u32	mode_link;
extern u8* kb_rf_pkt;
extern rf_packet_keyboard_t	pkt_km;
extern u8 host_keyboard_status;
extern int	km_dat_sending;
extern void irq_handler(void);
extern void kb_rf_proc(u32 key_scaned);
extern void main_loop(void);
extern void proc_suspend(void);
extern int  rf_rx_process(u8 * p);
extern void tick_8ms_adjust(int adjust);
extern void user_init(void);
extern void kb_rf_init(kb_status_t *kb_status);
extern void kb_pairing_mode_detect(void);
extern void kb_paring_and_syncing_proc(kb_status_t *kb_status);


static inline led_cfg_t kb_led_pairing_end_cfg_cust( u32 pairing_end )
{
     if ( pairing_end == p_kb_custom_cfg->led_pairing_end_mode ){
         kb_led_cfg[E_LED_PAIRING_END].on_time = kb_led_cfg[E_LED_PAIRING_END].off_time;
     }
     return kb_led_cfg[E_LED_PAIRING_END];
}

#endif /* KB_RF_H_ */
