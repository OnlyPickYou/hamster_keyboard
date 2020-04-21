/*
 * kb_custom.h
 *
 *  Created on: Sep 1, 2014
 *      Author: qcmao
 */

#ifndef KB_CUSTOM_H_
#define KB_CUSTOM_H_

#include "../common/device_led.h"
#include "../../proj/config/user_config.h"

//offset: 0x3f00
typedef struct{
	u16	pipe_pairing;			//0-1, pipe 0,pairig
	u16 pipe_kb_data;			//2-3, pipe 2,kb data
	u32 did;					//4-7 device id
	u8	cap;					//8 crystal CAP setting
	u8  led_pairing_end_mode;   //9
	u8  tx_power_paring;		//a
    u8  tx_power;   	 		//b
	u8  tx_power_emi;			//c
	u8  kb_lock_flg; 			//d, keyboard lock flag
	u8  kb_pon_led_bhv;			//e, keyboard power on led behavior
	u8  kb_size_sel;			//f, keyboard size select
	u16 kb_pairing_key; 		//10-11, pairing key for keyboard

	u8	deep_slp_tick;          //12  time into deep_sleep 1s unit

}kb_custom_cfg_t;

enum{
	E_LED_POWER_ON = 0,
	E_LED_AUTO_PAIRING,		//1
    E_LED_MANUAL_PAIRING,	//2
    E_LED_PAIRING_END,		//3
    E_LED_BAT_LOW,			//4
    E_LED_EMI,				//5
    E_LED_OTA,				//6
    E_LED_RSVD,				//7
};

extern led_cfg_t kb_led_cfg[];
extern kb_custom_cfg_t *p_kb_custom_cfg;
extern void kb_custom_init(kb_status_t *p_kb_status);

#define kb_cust_tx_power         (p_kb_custom_cfg->tx_power)
#define GET_HOST_ACCESS_CODE_FLASH_OTP  (p_kb_custom_cfg->pipe_kb_data)
#define kb_cust_tx_power_paring  ( (p_kb_custom_cfg->tx_power_paring == 0xff) ? RF_POWER_m24dBm : p_kb_custom_cfg->tx_power_paring )
#define kb_cust_tx_power_emi	((p_kb_custom_cfg->tx_power_emi == 0xff) ? RF_POWER_8dBm : p_kb_custom_cfg->tx_power_emi)

#endif /* KB_CUSTOM_H_ */
