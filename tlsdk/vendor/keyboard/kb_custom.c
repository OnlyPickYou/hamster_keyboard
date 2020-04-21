/*
 * kb_custom.c
 *
 *  Created on: Sep 1, 2014
 *      Author: qcmao
 */
#include "../../proj/tl_common.h"
#include "kb_custom.h"
#include "../common/default_config.h"
#include "kb_rf.h"
#include "../common/rf_frame.h"
#include "../../proj_lib/rf_drv_8266.h"

kb_custom_cfg_t *p_kb_custom_cfg;
extern kb_pair_info_t kb_pair_info;
extern rf_packet_pairing_t	pkt_pairing;
//on_time, off_time, repeat_count, over_wrt
led_cfg_t kb_led_cfg[] = {
    {32,     1,      1,      0x40},    //power-on, 2s on
    {4,		 4,		 20,	 0x40},    //pairing auto, 2Hz, slow blink
    {1,      1,      20,     0x80},    //pairing manual, 8Hz, fast blink
    {0,      8,      3,      0x80},    //pairing end
    {4,      4,      3,      0},       //battery low  2Hz
    {1,      1,      20,     0x80},    //EMI, 1Hz
    {10,     10,     10,      0x80},   //ota
    {2,      4,      3,      0}        //rsvd, 3Hz
};
void kb_custom_init(kb_status_t *p_kb_status)
{
	p_kb_custom_cfg = (kb_custom_cfg_t *)DEVICE_ID_ADDRESS;//3f00

    if(p_kb_custom_cfg->pipe_pairing != U16_MAX){
		rf_set_access_code0 (rf_access_code_16to32(p_kb_custom_cfg->pipe_pairing));
	}

	if (p_kb_custom_cfg->did != U32_MAX) {
		pkt_pairing.did = p_kb_custom_cfg->did;
		p_kb_status->device_id = p_kb_custom_cfg->did;
	}
	else {
		p_kb_status->device_id = pkt_pairing.did;
	}

	if(p_kb_custom_cfg->kb_lock_flg != U8_MAX){
		//default is no lock
	}
	if(p_kb_custom_cfg->kb_pon_led_bhv != U8_MAX){
		//default behaviour is ?
	}
	if(p_kb_custom_cfg->kb_size_sel != U8_MAX){
		//default kb size is large or small
	}
	if(p_kb_custom_cfg->kb_pairing_key != U16_MAX){
		kb_pair_info.paring_key_1 = p_kb_custom_cfg->kb_pairing_key >> 8;
		kb_pair_info.paring_key_2 = p_kb_custom_cfg->kb_pairing_key & 0xff;
	}
}
