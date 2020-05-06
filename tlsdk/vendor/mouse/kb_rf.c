/*
 * kb_rf.c
 *
 *  Created on: 2015-1-21
 *      Author: Administrator
 */

#include "../../proj/tl_common.h"
#include "../../proj_lib/pm.h"
#include "../../proj_lib/rf_drv.h"
#include "../common/rf_frame.h"

#include "kb_custom.h"
#include "kb_led.h"
#include "kb_rf.h"
#include "kb.h"
//#include "kb_pm.h"

extern  kb_status_t kb_status;

#define HOST_NO_LINK        (kb_status.no_ack >= 400)
#define HOST_LINK_LOST		(kb_status.no_ack >= 300)


u32		tick_last_tx;

int		km_dat_sending = 0;


#if(!KEYBOARD_PIPE1_DATA_WITH_DID)
u8 pipe1_send_id_flg = 0;
#endif



u8* kb_rf_pkt = (u8*)&pkt_pairing;



extern kb_data_t	kb_event;

//02(50/10)510b
rf_packet_pairing_t	pkt_pairing = {
		sizeof (rf_packet_pairing_t) - 4,	// 0x0c=16-4,dma_len
#if RF_FAST_MODE_1M
		RF_PROTO_BYTE,
		sizeof (rf_packet_pairing_t) - 6,	// rf_len
#else
		sizeof (rf_packet_pairing_t) - 5,	// 0x0b=16-5,rf_len
		RF_PROTO_BYTE,						// 0x51,proto
#endif
		PKT_FLOW_DIR,						// flow, pairing type: auto(0x50) or manual(0x10)
		FRAME_TYPE_KEYBOARD,				// 0x02,type

//		PIPE0_CODE,			// gid0

		0,					// rssi
		0,					// per
		0,					// seq_no
		0,					// reserved
		0xdeadbeef,			// device id
};




//0280510f
rf_packet_keyboard_t	pkt_km = {
		sizeof (rf_packet_keyboard_t) - 4,	//0x10=20-4,dma_len

		sizeof (rf_packet_keyboard_t) - 5,	//0x0f=20-5,rf_len
		RF_PROTO_BYTE,						// 0x51, proto
		PKT_FLOW_DIR,						// 0x80, kb data flow
		FRAME_TYPE_KEYBOARD,				// 0x02, type

//		U32_MAX,			// gid0

		0,					// rssi
		0,					// per
		0,					// seq_no
		1,					// pno
};

int kb_pairing_mode_detect(void)
{
	//if power on detect combined key is ESC+Q
	if (   (VK_ESC == kb_event.keycode[0]&& VK_Q   == kb_event.keycode[1] && 2 == kb_event.cnt) \
		|| (VK_Q   == kb_event.keycode[0]&& VK_ESC == kb_event.keycode[1] && 2 == kb_event.cnt) ){
		return 1;
	}
	return 0;
}





extern rf_packet_pairing_t	pkt_pairing;



void kb_rf_init(void)
{
	ll_device_init ();
	rf_receiving_pipe_enble(0x3f);	// channel mask
	kb_status.tx_retry = 5;

    if(kb_status.kb_mode == STATE_NORMAL){  //link OK deep back
    	rf_set_access_code1 (kb_status.dongle_id);
    	kb_rf_pkt = (u8*)&pkt_km;
    	rf_set_tx_pipe (PIPE_KEYBOARD);
    	rf_set_power_level_index (kb_cust_tx_power);
    }
    else{ //poweron or link ERR deepback
    	rf_set_access_code1 ( U32_MAX );
    	rf_set_tx_pipe (PIPE_PARING);
    	rf_set_power_level_index (kb_cust_tx_power_paring);
    }
}


extern u32 key_scaned;
void kb_paring_and_syncing_proc(void)
{
    if( (kb_status.mode_link&LINK_PIPE_CODE_OK) && kb_rf_pkt != (u8*)&pkt_km ){ //if link on,change to KB data pipe   kb_mode to STATE_NORMAL
    	kb_rf_pkt = (u8*)&pkt_km;
    	rf_set_tx_pipe (PIPE_KEYBOARD);
    	kb_status.tx_retry = 2;
    	rf_set_power_level_index (kb_cust_tx_power);
    	if(kb_status.kb_mode == STATE_PAIRING){
    		kb_device_led_setup( kb_led_cfg[KB_LED_PAIRING_OK] );
    	}

    	kb_status.kb_mode = STATE_NORMAL;
    	memset(&kb_event,0,sizeof(kb_event));
    	key_scaned = 1;
    }

	if( kb_status.kb_mode == STATE_PAIRING ){  //manual paring
    	//pkt_pairing.flow = PKT_FLOW_PARING;
        if( kb_status.loop_cnt >= KB_MANUAL_PARING_MOST ){ //pairing timeout,change to syncing mode
            kb_status.kb_mode = STATE_SYNCING;
            pkt_pairing.flow = PKT_FLOW_TOKEN;
            kb_device_led_setup( kb_led_cfg[KB_LED_PAIRING_END] );
            rf_set_power_level_index (kb_cust_tx_power);
        }
    }
	else if ( kb_status.kb_mode == STATE_SYNCING){
		if(kb_status.loop_cnt < KB_PARING_POWER_ON_CNT){
			pkt_pairing.flow = PKT_FLOW_TOKEN | PKT_FLOW_PARING;
		}
		else if(kb_status.loop_cnt == KB_PARING_POWER_ON_CNT){
			pkt_pairing.flow = PKT_FLOW_TOKEN;
			rf_set_power_level_index (kb_cust_tx_power);
		}

		if( kb_pairing_mode_detect() ){
			//SET_PARING_ENTERED();
			pkt_pairing.flow = PKT_FLOW_PARING;
			kb_device_led_setup(kb_led_cfg[KB_LED_MANUAL_PAIRING]);	//8Hz,fast blink
			kb_status.kb_mode  = STATE_PAIRING;
			rf_set_power_level_index (kb_cust_tx_power_paring);
			kb_status.loop_cnt = KB_PARING_POWER_ON_CNT;
		}
	}
}


////////////////////////////////////////////////////////////////////////
//	keyboard/mouse data management
////////////////////////////////////////////////////////////////////////



u32 rx_rssi = 0;
u8 	rssi_last;
inline void cal_pkt_rssi(void)
{
#if(DBG_ADC_DATA)
	kb_status.tx_retry = 2;
#else
	if(kb_status.kb_pipe_rssi){
		if(!rx_rssi){
			rx_rssi = kb_status.kb_pipe_rssi;
		}
		else{
			if ( abs(rssi_last - kb_status.kb_pipe_rssi) < 8 ){
				rx_rssi = (rx_rssi*3 + kb_status.kb_pipe_rssi ) >> 2;
			}
		}
		rssi_last = kb_status.kb_pipe_rssi;
		pkt_km.rssi = rx_rssi;

		kb_status.kb_pipe_rssi = 0;  //clear


		//0x32: -60  0x28: -70    0x23: -75   0x1d: -80
		if(pkt_km.rssi < 0x1d){   // < -80
			kb_status.tx_retry = 7;
		}
		else if(pkt_km.rssi < 0x28){  //-70
			kb_status.tx_retry = 5;
		}
		else if(pkt_km.rssi < 0x32){  //-60
			kb_status.tx_retry = 3;
		}
		else{
			kb_status.tx_retry = 2;
		}

#if(LOW_TX_POWER_WHEN_SHORT_DISTANCE)
		if(pkt_km.rssi < 0x35){  //-57   power low
			if(kb_status.tx_power == RF_POWER_2dBm){
				rf_set_power_level_index (kb_status.cust_tx_power);
				kb_status.tx_power = kb_status.cust_tx_power;
			}
		}
		else if(kb_status.tx_power == kb_status.cust_tx_power){  //power high
			rf_set_power_level_index (RF_POWER_2dBm);
			kb_status.tx_power = RF_POWER_2dBm;
		}
#endif
	}
#endif
}


void kb_rf_proc( u32 key_scaned )
{
	static u32 kb_tx_retry_thresh = 0;

	if (kb_status.mode_link ) {
		if (clock_time_exceed (tick_last_tx, 1000000)) {//1s
			kb_status.host_keyboard_status = KB_NUMLOCK_STATUS_INVALID;
		}
        if ( key_scaned ){
			memcpy ((void *) &pkt_km.data[0], (void *) &kb_event, sizeof(kb_data_t));
			pkt_km.seq_no++;
			km_dat_sending = 1;
			kb_tx_retry_thresh = 0x400;
		}

#if(!KEYBOARD_PIPE1_DATA_WITH_DID)
    	//fix auto paring bug, if dongle ACK ask for  id,send it in on pipe1
        int allow_did_in_kb_data = 0;
		if(pipe1_send_id_flg){
			if(key_scaned){ //kb data with did in last 4 byte
				if(kb_event.cnt < 3){
	        		allow_did_in_kb_data = 1;
				}
			}
			else{  //no kb data, only did in last 4 byte; seq_no keep same, so dongle reject this invalid data
				allow_did_in_kb_data = 1;
			}

			if(allow_did_in_kb_data){
				*(u32 *) (&pkt_km.data[4]) = pkt_pairing.did;  //did in last 4 byte
				pkt_km.type = FRAME_TYPE_KB_SEND_ID;
				km_dat_sending = 1;
				kb_tx_retry_thresh = 0x400;
			}
			else{
				pkt_km.type = FRAME_TYPE_KEYBOARD;
			}
		}
		else{
			 pkt_km.type = FRAME_TYPE_KEYBOARD;
		}
#endif

		if (km_dat_sending) {
            if ( kb_tx_retry_thresh-- == 0 ){
				km_dat_sending = 0;
            }
		}
	}
	else{
		pkt_pairing.seq_no++;
	}

	kb_status.rf_sending = ((km_dat_sending || !kb_status.mode_link) && (kb_status.kb_mode != STATE_WAIT_DEEP));
	if(kb_status.rf_sending){
		if(HOST_LINK_LOST && kb_status.mode_link){
			kb_status.tx_retry = 5;
		}
		if(device_send_packet ( kb_rf_pkt, 550, kb_status.tx_retry, 0) ){
			km_dat_sending = 0;
			tick_last_tx = clock_time();
			kb_status.no_ack = 0;
		}
		else{
			kb_status.no_ack ++;
#if(!DBG_ADC_DATA)
			pkt_km.per ++;
#endif
		}

		cal_pkt_rssi();
	}
}

u8	mode_link = 0;
_attribute_ram_code_ int  rf_rx_process(u8 * p)
{
	rf_packet_ack_pairing_t *p_pkt = (rf_packet_ack_pairing_t *) (p + 8);
	static u8 rf_rx_mouse;
	if (p_pkt->proto == RF_PROTO_BYTE) {
		pkt_pairing.rssi = p[4];
		pkt_km.rssi = p[4];
        //pkt_km.per ^= 0x80;
		///////////////  Paring/Link ACK //////////////////////////
		if (p_pkt->type == FRAME_TYPE_ACK && (p_pkt->did == pkt_pairing.did) ) {	//paring/link request
            rf_set_access_code1 (p_pkt->gid1);          //access_code1 = p_pkt->gid1;
			mode_link = 1;
			return 1;
		}
		////////// end of PIPE1 /////////////////////////////////////
		///////////// PIPE1: ACK /////////////////////////////
		else if (p_pkt->type == FRAME_TYPE_ACK_MOUSE) {
			rf_rx_mouse++;
#if(!MOUSE_PIPE1_DATA_WITH_DID)
			pipe1_send_id_flg = 0;
#endif
			return 1;
		}
#if(!MOUSE_PIPE1_DATA_WITH_DID)
		else if(p_pkt->type == FRAME_AUTO_ACK_MOUSE_ASK_ID){ //fix auto bug
			pipe1_send_id_flg = 1;
			return 1;
		}
#endif

		////////// end of PIPE1 /////////////////////////////////////
	}
	return 0;
}
