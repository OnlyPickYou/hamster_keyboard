/*
 * kb_custom.c
 *
 *  Created on: 2015-1-21
 *      Author: Administrator
 */

#include "../../proj/tl_common.h"
#include "../../proj_lib/pm.h"
#include "../../proj_lib/rf_drv.h"
#include "../common/rf_frame.h"

#include "kb_default_config.h"
#include "kb_custom.h"
#include "kb_rf.h"


kb_custom_cfg_t *p_kb_custom_cfg;

/******************************************************************
 * function:
 * normal_addr -> 0x3c00  fn_addr -> 0x3c90
 * numlock_addr -> 0x3d20 fn_numlock_addr -> 0x3db0
 *
 * if you need to fix the cust addr then
 * normal_addr -> 0x3900  fn_addr -> 0x3990
 * numlock_addr -> 0x3920 fn_numlock_addr -> 0x39b0
 *
*****************************************************************/


void kb_custom_init(kb_status_t *kb_status)
{
	//3f00  3f30  3f60
	for(int i=0;i<3;i++){
		p_kb_custom_cfg = (kb_custom_cfg_t *) (DEVICE_ID_ADDRESS+(i*0x30));
		if(p_kb_custom_cfg->cap != 0){
			break;
		}
	}

    if(p_kb_custom_cfg->pipe_pairing != U16_MAX){
		rf_set_access_code0 (rf_access_code_16to32(p_kb_custom_cfg->pipe_pairing));
	}

	if (p_kb_custom_cfg->did != U32_MAX) {
		pkt_pairing.did = p_kb_custom_cfg->did;
	}
#if (KEYBOARD_PIPE1_DATA_WITH_DID)
	pkt_km.did = pkt_pairing.did;
#endif

	analog_write(0x81,p_kb_custom_cfg->cap == U8_MAX ? 0xd8 : p_kb_custom_cfg->cap);

	kb_status->cust_tx_power = (p_kb_custom_cfg->tx_power == 0xff) ? RF_POWER_8dBm : p_kb_custom_cfg->tx_power;
	kb_status->tx_power = RF_POWER_8dBm;

	kb_status->led_gpio_lvd = (p_kb_custom_cfg->gpio_lvd == U16_MAX) ? 0 : p_kb_custom_cfg->gpio_lvd;

	kb_status->led_gpio_scr = (p_kb_custom_cfg->gpio_scr == U16_MAX) ? 0 : p_kb_custom_cfg->gpio_scr;
	kb_status->led_gpio_cap = (p_kb_custom_cfg->gpio_cap == U16_MAX) ? 0 : p_kb_custom_cfg->gpio_cap;
	kb_status->led_gpio_num = (p_kb_custom_cfg->gpio_num == U16_MAX) ? 0 : p_kb_custom_cfg->gpio_num;

	kb_status->led_level_lvd = (p_kb_custom_cfg->level_lvd == U8_MAX) ? 1 : 0; //0xff:high_valid,level = 1

	kb_status->led_level_scr = (p_kb_custom_cfg->level_scr == U8_MAX) ? 0 : 1; //0xff:high valid,level = 0
	kb_status->led_level_cap = (p_kb_custom_cfg->level_cap == U8_MAX) ? 0 : 1; //0xff:high valid,level = 0
	kb_status->led_level_num = (p_kb_custom_cfg->level_num == U8_MAX) ? 0 : 1; //0xff:high valid,level = 0

}

