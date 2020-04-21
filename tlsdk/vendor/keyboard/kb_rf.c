#include "../../proj/tl_common.h"
#include "../../proj/mcu/watchdog_i.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../common/rf_frame.h"
#include "../common/rf_ota.h"
#include "../link_layer/rf_ll.h"

#include "kb_rf.h"
#include "kb_custom.h"
#include "kb_device_info.h"

#define			PARING_KEY_MAX				2
u8			host_keyboard_status = 0;//KB_NUMLOCK_STATUS_INVALID;
u32			mode_link = 0;
u32			access_code1;

kb_pair_info_t kb_pair_info = {
		VK_ESC,			//0x29
		VK_Q,			//0x14
		PARING_KEY_MAX,	//2
		0,
		0,
		0,

		0,
		0
};

extern kb_data_t 	kb_data;
extern kb_data_t	kb_event;
extern kb_status_t	kb_status;

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
u8* kb_rf_pkt = (u8*)&pkt_pairing;

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
		1,					// number of frame
};

void kb_pairing_mode_detect(void)
{
//if power on detect combined key is ESC+Q
	if ((kb_pair_info.paring_key_1 == kb_event.keycode[0]&& \
			kb_pair_info.paring_key_2 == kb_event.keycode[1] && \
			kb_pair_info.paring_key_cnt == kb_event.cnt)
		|| (kb_pair_info.paring_key_2 == kb_event.keycode[0] && \
			kb_pair_info.paring_key_1 == kb_event.keycode[1] && \
			kb_pair_info.paring_key_cnt == kb_event.cnt)
	/*|| ( KB_PAIRING_BTN_EN && proc_button_pairing_detect() ) */){

		kb_led_setup(kb_led_cfg[E_LED_MANUAL_PAIRING]);	//8Hz,fast blink
		kb_status.kb_mode  = STATE_PAIRING;
	}
}

void kb_rf_init(kb_status_t *kb_status)
{
	// pkt_kb buffer init
	ll_device_init ();
	rf_receiving_pipe_enble(0x3f);	// channel mask
    kb_status->pkt_addr = (u32)&pkt_km;
    u32 tx_power_dft = RF_POWER_8dBm;
    kb_status->tx_retry = 2;
    kb_status->tx_power = (kb_cust_tx_power == U8_MAX) ? tx_power_dft : kb_cust_tx_power;


    if(kb_status->kb_mode == STATE_NORMAL){  //link OK deep back
    	mode_link = 1;
    	rf_set_access_code1 (kb_status->dongle_id);
    	kb_rf_pkt = (u8*)&pkt_km;
    	rf_set_tx_pipe (PIPE_KEYBOARD);
    	rf_set_power_level_index (kb_status->tx_power);
    }
    else{ //poweron or link ERR deepback
    	rf_set_access_code1 ( U32_MAX );
    	rf_set_tx_pipe (PIPE_PARING);
    	rf_set_power_level_index (kb_cust_tx_power_paring);
    }
}



void kb_paring_and_syncing_proc(kb_status_t *kb_status)
{
    if(mode_link == 1 && kb_rf_pkt != (u8*)&pkt_km ){ //if link on,change to KB data pipe   kb_mode to STATE_NORMAL
    	kb_rf_pkt = (u8*)&pkt_km;
    	rf_set_tx_pipe (PIPE_KEYBOARD);
    	rf_set_power_level_index (kb_status->tx_power);
    	kb_status->kb_mode = STATE_NORMAL;
    }

	if( kb_status->kb_mode == STATE_PAIRING ){  //manual paring
    	pkt_pairing.flow = PKT_FLOW_PARING;

        if( kb_status->loop_cnt >= KB_MANUAL_PARING_MOST ){ //pairing timeout,change to syncing mode
            kb_status->kb_mode = STATE_SYNCING;
            pkt_pairing.flow = PKT_FLOW_TOKEN;
            rf_set_power_level_index (kb_status->tx_power);
        }
    }
	else if ( kb_status->kb_mode == STATE_SYNCING){
		if(kb_status->loop_cnt < KB_PARING_POWER_ON_CNT){
			pkt_pairing.flow = PKT_FLOW_TOKEN | PKT_FLOW_PARING;
		}
		else if(kb_status->loop_cnt == KB_PARING_POWER_ON_CNT){
			pkt_pairing.flow = PKT_FLOW_TOKEN;
			rf_set_power_level_index (kb_status->tx_power);
		}
	}

    kb_status->loop_cnt++;
}

_attribute_ram_code_ int  rf_rx_process(u8 * p)
{
	rf_packet_ack_pairing_t *p_pkt = (rf_packet_ack_pairing_t *) (p + 8);
	static u32 rf_rx_keyboard;
	if (p_pkt->proto == RF_PROTO_BYTE) {
		pkt_pairing.rssi = p[4];
		pkt_km.rssi = p[4];
		///////////////  Paring/Link ACK //////////////////////////
		if ( p_pkt->type == FRAME_TYPE_ACK && (p_pkt->did == pkt_pairing.did) ) {	//paring/link request
			//change to pip2 STATE_NORMAL
			rf_set_access_code1 (p_pkt->gid1);//need change to pipe2 that is for kb's data
			mode_link = 1;
			return 1;
		}
		////////// end of PIPE1 /////////////////////////////////////
		///////////// PIPE1: ACK /////////////////////////////
		else if (p_pkt->type == FRAME_TYPE_ACK_KEYBOARD) {
			rf_rx_keyboard++;
			host_keyboard_status =((rf_packet_ack_keyboard_t*)p_pkt)->status;
			return 1;
		}
		////////// end of PIPE1 /////////////////////////////////////
	}
	else if ( p_pkt->proto == RF_PROTO_OTA)
	{
		write_reg8(0x808000,0xbb);
		kb_rf_ota_data_rsv( (rf_packet_ota_ack_data_t *)(p+8) );
		return 0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////
//	keyboard/mouse data management
////////////////////////////////////////////////////////////////////////
int	km_dat_sending = 0;

u32		tick_last_tx;
u8 		kb_rf_send;
extern kb_status_t kb_status;
_attribute_ram_code_ void kb_rf_proc( u32 key_scaned )
{

	static u32 kb_tx_retry_thresh = 0;

	kb_status.rf_mode = RF_MODE_IDLE;
	if (mode_link) {
		if (clock_time_exceed (tick_last_tx, 1000000)) {//1s
			host_keyboard_status = KB_NUMLOCK_STATUS_INVALID;
		}
        if ( key_scaned ){
			pkt_km.pno = 1;
			memcpy ((void *) &pkt_km.data[0], (void *) &kb_event, sizeof(kb_data_t));
			pkt_km.seq_no++;
			km_dat_sending = 1;
			kb_tx_retry_thresh = 0x400;
		}

		if (km_dat_sending) {
			kb_status.rf_mode = RF_MODE_DATA;
            if ( kb_tx_retry_thresh-- == 0 ){
				km_dat_sending = 0;
            }
		}
	}
	else {
		kb_status.rf_mode = RF_MODE_SYNC;
        pkt_pairing.seq_no++;
	}


	kb_rf_send = DEVICE_PKT_ASK || mode_link == 0 ;
	if(kb_rf_send){
		if(device_send_packet ( kb_rf_pkt, 550, kb_status.tx_retry, 0) ){
			km_dat_sending = 0;
			tick_last_tx = clock_time();
			kb_status.no_ack = 0;
		}
		else if(kb_status.no_ack < KB_RF_SYNC_PKT_TH_NUM ){
			kb_status.no_ack ++;
			pkt_km.per ++;
		}
		else{
			pkt_km.per ++;
		}
	}
}
