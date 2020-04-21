/*
 * kb_emi.c
 *
 *  Created on: Sep 1, 2014
 *      Author: qcmao
 */

//#include "mouse.h"
//#include "mouse_button.h"
//#include "mouse_wheel.h"
//#include "mouse_twheel.h"
#include "../../proj/tl_common.h"
#include "../../proj/mcu/watchdog_i.h"
#include "../../proj_lib/rf_drv.h"
#include "../common/emi.h"
#include "kb_emi.h"
#include "kb_custom.h"
#include "kb_rf.h"

static u8 button_last;  //current button
static u8 button_pre;   //history button

extern kb_pair_info_t kb_pair_info;
extern kb_data_t	kb_event;

led_cfg_t kb_led_emi_cfg[] = {
    {1,      2,      2,      0x80},    //Carrier
    {4,      8,      2,      0x80},    //Carrier & Data
    {0,      1,      2,      0x80},    //RX
    {2,      0,      2,      0x80}     //TX
};

kb_emi_info_t kb_emi_info = {
		VK_ESC,

		VK_F1,
		VK_F2,
		VK_F3,
		VK_F4,

		VK_F5,
		VK_F6,
		VK_F7,
		VK_F8,

		VK_F9,
		VK_F10,
		VK_F11,
		VK_F12,

		0,0,0
};

static void kb_proc_button (void)
{
	u8 button = 0;
	u8 *p_emi = (u8 *) &kb_emi_info;

	foreach(i,sizeof(kb_emi_info)){
		if((kb_event.keycode[0] == *(p_emi + i )) && (kb_event.cnt == 1)){
				button_last = kb_event.keycode[0];
		}
	}

#if (DEBUG_FH)
	if ((button_last & 1) && !(button & 1))
		fh_disable_left = 40;
	if ((button_last & 2) && !(button & 2))
		fh_disable_right = 40;
#endif

#if (DEBUG_WATCH_DOG)
	if (button & FLAG_BUTTON_MIDDLE)
		while (1);
#endif
}

u32 kb_button_process_emi(s8 *chn_idx, u8 *test_mode_sel, u32 btn_pro_end)  //zjs need debug
{
	u32 cmd = 0;
	if((button_last != button_pre) && (button_last >= 0x3a ) && (button_last <= 0x45))		//new event
	{
		cmd = 0x80;
		button_last -= 0x3a;
		*test_mode_sel = (button_last) & 3;
		*chn_idx = (button_last) >> 2;
	}

	if(btn_pro_end)
		button_pre = button_last;
	return cmd;
}

void kb_emi_process(kb_status_t *kb_status)
{
	static s8   test_chn_idx = 1;
    static u8   test_mode_sel = 0;
    static u8   flg_emi_init = 0;

	//kb_button_detect( kb_status );
    kb_proc_button();
    u32 cmd = 0;


    if( 0 && kb_status->dbg_mode & STATE_TEST_EMI_BIT ){
        cmd = kb_button_process_emi( &test_chn_idx, &test_mode_sel, 0 );
        //kb_button_process(kb_status);
        if( !(kb_status->dbg_mode & STATE_TEST_EMI_BIT) ){    //recover to emi rx mode
            cmd = 1;
            test_mode_sel = 2;
        }
    }
    else{
       cmd = kb_button_process_emi( &test_chn_idx, &test_mode_sel, 1 );
    }
    cmd |= !flg_emi_init;
    if( !flg_emi_init ){
        flg_emi_init = 1;
    }
    kb_led_setup( kb_led_emi_cfg[test_mode_sel] );
	kb_led_process( kb_status->led_define );
	emi_process( cmd , test_chn_idx, test_mode_sel, kb_status->pkt_addr, kb_cust_tx_power_emi );
}
