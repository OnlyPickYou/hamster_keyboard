/*
 * kb_ota.c
 *
 *  Created on: 2014-11-18
 *      Author: Telink
 */

#include "../../proj/tl_common.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../common/rf_frame.h"
#include "../common/rf_ota.h"
#include "../link_layer/rf_ll.h"

#include "kb_ota.h"
#include "kb_custom.h"

volatile u32 	ota_adr = 0;
volatile u32    ota_status = 0;
volatile int 	ota_retry = 0;

volatile u8  	ota_seq = 0;
volatile u8		ota_start = 0;
volatile u8		ota_rsp = 0;
volatile u8     ota_rf_data_ok = 0;


volatile u32 debug_ota_ack_proto;
rf_packet_ota_ack_data_t *pkt_rsv;

extern kb_data_t	kb_event;


rf_packet_ota_req_data_t pkt_ota_req = {
		sizeof (rf_packet_ota_req_data_t) - 4,

		sizeof (rf_packet_ota_req_data_t) - 5,
		RF_PROTO_OTA,
		FLG_RF_OTA_DATA,
		0,  //seq

		0,  //adr
};

void kb_ota_init(void)
{
}


_attribute_ram_code_ void start_reboot(void)
{
	irq_disable ();
	analog_write(0x3a,0);
	REG_ADDR8 (0x602) = 0x84;
	while(1);
}

void kb_ota_mode_detect(kb_status_t *kb_status) //_attribute_ram_code_
{
#if(KB_OTA_ENABLE)
	static u32 ota_cmd_half_time;
	if(ota_status == KB_OTA_ST_NONE && kb_status->kb_mode == STATE_NORMAL){
		if(KB_KEY_XYZ){
			ota_status = KB_OTA_ST_CMD_HALF;
			ota_cmd_half_time = clock_time();
		}
	}
	else if(ota_status == KB_OTA_ST_CMD_HALF){
		if(KB_KEY_OPQ){
			ota_status = KB_OTA_ST_BEGIN;
			kb_status->kb_mode = STATE_OTA;
		}
		else if(KB_KEY_MN){  //debug
			start_reboot();
		}
		if(clock_time_exceed(ota_cmd_half_time,2000000)){
			ota_status = KB_OTA_ST_NONE;
		}
	}
#endif
}


void kb_ota_data_pre_proc(void)
{
	static u32 ota_no;
	ota_no++;

	ota_rf_data_ok = 0;

	if (!ota_start)
	{
		ota_start = 1;
		ota_adr = 0;
		ota_seq = 0;
		ota_retry = 0;
		ota_rsp = 0;
	}

	if (ota_rsp)
	{
		ota_retry = 0;
		ota_adr += 32;
		ota_seq++;
	}
	else
	{
		ota_retry ++;
	}

	if (ota_start == 2 || ota_retry > 64)		// terminate
	{
			ota_start = 0;
	}
	else
	{
		pkt_ota_req.seq = ota_seq;
		pkt_ota_req.adr = ota_adr;
		ota_rsp = 0;
	}
}


volatile u32 ota_dbg_adr;
volatile u16 ota_dbg_crc;
volatile u32 ota_dbg_good_data;

_attribute_ram_code_ void kb_ota_data_post_proc (rf_packet_ota_ack_data_t *p)
{
	///////////  data qualification //////////////////////
	ota_dbg_adr = p->adr;

	if(p->adr == U32_MAX && p->crc == U16_MAX){
		ota_rsp = 1;
		ota_start = 2;								//indicate end
		flash_erase_sector (0x1f000);
		u32 flag = 0xa5;
		flash_write_page (0x1f000, 4, &flag);
		start_reboot();
	}
	else if(p->adr == ota_adr){
		u16 cur_crc = crc16(&p->adr, 36);
		ota_dbg_crc = cur_crc;
		if(cur_crc == p->crc ){
			ota_rsp = 1;
			ota_dbg_good_data++;

			if ((ota_adr & 0xfff)==0){
				flash_erase_sector (0x10000 + ota_adr);
			}
			flash_write_page (0x10000 + ota_adr, 32, p->dat);
		}
	}
}



_attribute_ram_code_ void kb_rf_ota_data_rsv(rf_packet_ota_ack_data_t *p)
{
#if(KB_OTA_ENABLE)
	extern int		device_ack_received;
	if (p->flag == FLG_RF_OTA_DATA && (p->adr == ota_adr || p->adr == U32_MAX) )
	{
		device_ack_received = 1;
		ota_rf_data_ok = 1;
		pkt_rsv = p;
	}
#endif
}


void kb_ota_process(kb_status_t *kb_status)
{
#if(KB_OTA_ENABLE)

	dbg_led_high;
	kb_ota_data_pre_proc();

	dbg_led_low;
	if(device_send_packet ( &pkt_ota_req, 600, 2, 123) );
	dbg_led_high;

	kb_led_setup( kb_led_cfg[E_LED_OTA] );
	kb_led_process(kb_status->led_define);
	ll_add_clock_time (8000);

	if(ota_rf_data_ok){
		kb_ota_data_post_proc(pkt_rsv);
	}

#if(0)
	static u32 cpu_wakeup_tick;
	while (!clock_time_exceed (cpu_wakeup_tick, 8000));
	cpu_wakeup_tick = clock_time ();
#else
	u32 next_wakeup_tick = reg_system_wakeup_tick + CLOCK_SYS_CLOCK_1MS * 8;
	if ( next_wakeup_tick < clock_time() + CLOCK_SYS_CLOCK_1MS * 2 )
		next_wakeup_tick = clock_time() + CLOCK_SYS_CLOCK_1MS * 8;
	kb_cpu_sleep_wakeup (0, PM_WAKEUP_TIMER, next_wakeup_tick);
#endif

#endif
}
