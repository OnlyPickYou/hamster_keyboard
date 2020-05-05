#include "../../proj/tl_common.h"
#include "../../proj/mcu/watchdog_i.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../common/rf_frame.h"
#include "../common/emi.h"

#include "device_info.h"
#include "mouse.h"
#include "mouse_rf.h"
#include "mouse_batt.h"
#include "mouse_rf.h"
#include "mouse_custom.h"

custom_cfg_t   *p_custom_cfg;

const u32 m_hw_def_dft[] = {
    M_HW_BTN_LEFT,
    M_HW_BTN_RIGHT,
    M_HW_BTN_MIDL,
    M_HW_BTN_BB,
    M_HW_BTN_FB,
    M_HW_BTN_CPI,
    M_HW_LED_CTL,    

    M_HW_GPIO_LEVEL_LEFT | (M_HW_GPIO_LEVEL_RIGHT<<8) | (M_HW_GPIO_LEVEL_MIDL<<16) | (M_HW_GPIO_LEVEL_BB<<24),
    M_HW_GPIO_LEVEL_FB   | (M_HW_GPIO_LEVEL_CPI<<8)   | (M_HW_GPIO_LEVEL_LED<<16)  | (M_HW_VBAT_CHN<<24),

    M_HW_CFG_1_DIR_0,
    M_HW_CFG_1_DIR_1,
    M_HW_CFG_2_DIR_0,
    M_HW_CFG_2_DIR_1,
    

    M_HW_WHEEL_Z0,
    M_HW_WHEEL_Z1,
    M_HW_SNS_DATA,    
    M_HW_SNS_CLK,
    M_HW_SNS_MOT_PIN,
};



custom_btn_ui_t mouse_btn_ui = {
    ( FLAG_BUTTON_LEFT | FLAG_BUTTON_RIGHT ),                       //pairing ui
    ( FLAG_BUTTON_LEFT | FLAG_BUTTON_RIGHT | FLAG_BUTTON_MIDDLE ),  //emi ui
    ( FLAG_BUTTON_MIDDLE | FLAG_BUTTON_RIGHT ),                       //cpi-2-btn ui, should not be the same with pairing ui
    U8_MAX,                                                         //cpi-2-btn time
};


led_cfg_t mouse_led_cfg[] = {
    32,     1,      1,      0x40,    //power-on, 2s on
    2,      2,      255,    0x40,    //pairing manual, 4Hz
    0,      8,      3,      0x80,    //pairing end
    4,      4,      3,      0,       //battery low  2Hz
    8,      8,      3,      0,       //cpi, 1Hz
    0,      8,      3,      0,       //rsvd, 3Hz
};


#if 1//MOUSE_CUSTOM_FULL_FUNCTION    
void mouse_custom_re_get( u8 *p_dst, u8 *p_src_0, u8 *p_src_1, u32 len ){
    int i;
    for( i = 0; i < len; i++ ){
        *p_dst = (*p_src_1 == U8_MAX) ? *p_src_0 : *p_src_1;
        *p_src_0++;
        *p_src_1++;
        *p_dst++;
    }
}

void mouse_custom_re_get_4( u32 *p_dst, u32 *p_src_0, u32 *p_src_1, u32 len ){
    int i;
    for( i = 0; i < len; i++ ){
        *p_dst = (*p_src_1 == U32_MAX) ? *p_src_0 : *p_src_1;
        *p_src_0++;
        *p_src_1++;
        *p_dst++;
    }
}
#else
static inline void mouse_custom_re_get( u8 *p_dst, u8 *p_src_0, u8 *p_src_1, u32 len ) {}
static inline void mouse_custom_re_get_4( u32 *p_dst, u32 *p_src_0, u32 *p_src_1, u32 len ) {}
#endif

//cfg_init	to£ºval_c   =        0    |   1    |   2    |      3
//--------------------------------------------------------------
//	0		cust_addr_0 =        8    |   a    |   4    |      6
//	1		cust_addr_1 =        7    |   9    |   3    |      5
//	2		cust_addr_2 =        6    |   8    |   2    |      4
//	3		cust_addr_3 =        5    |   7    |   1    |      3
//--------------------------------------------------------------
u8 custom_cfg_re_define( u8 cfg, u8* p_cfg_re_def ){
	u32 val_c = ( *(p_cfg_re_def + cfg) == U8_MAX ) ? cfg : \
		( ( ( cfg + *(p_cfg_re_def+cfg) ) >> 1 ) & 3 );
	return val_c;
}



extern rf_packet_pairing_t	pkt_pairing;

void mouse_custom_init ( mouse_status_t *pStatus ){

	for(int i = 0; i<3; i++){
		p_custom_cfg = (custom_cfg_t *)(DEVICE_ID_ADDRESS - i*0x100);
		if(p_custom_cfg->cap != 0){
			break;
		}
	}

    pkt_pairing.did = (p_custom_cfg->did == U32_MAX) ? pkt_pairing.did : p_custom_cfg->did;   //device-id init

	u16 vendor_id = p_custom_cfg->vid;
    if(vendor_id != U16_MAX){
		rf_set_access_code0 (rf_access_code_16to32(vendor_id));
	}


}

