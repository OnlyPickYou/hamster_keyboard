/*
 * kb_device_info.c
 *
 *  Created on: Sep 1, 2014
 *      Author: qcmao
 */

#include "../../proj/tl_common.h"
#include "../../proj/mcu/watchdog_i.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../common/rf_frame.h"
#include "kb_device_info.h"
#include "kb_custom.h"
#include "kb_rf.h"

#define		PM_REG_START		0x34
#define		PM_REG_END			0x3a

#ifndef 	PM_POWERON_DETECTION_ENABLE
#define 	PM_POWERON_DETECTION_ENABLE 		0
#endif

static kb_device_info_t kb_device_info;

/*
 *  Base on from power on or deep sleep back
 *  Load customizable information from the 3.3V Analog register
 *  or Load from OTP
 *  kb_status: output parameter
 *
 */
void kb_device_info_load(kb_status_t *kb_status)
{
    u8 * pd = (u8 *) &kb_device_info;
    for (u8 i = PM_REG_START; i <= PM_REG_END; i++) {
        *pd ++ = analog_read (i);
    }

    kb_device_info.poweron = ((kb_device_info.mode^0x80) + 2) | 1;
    analog_write (PM_REG_END, kb_device_info.poweron);

    kb_status->kb_mode = kb_device_info.mode ? STATE_NORMAL : STATE_POWERON;

    if(kb_status->kb_mode == STATE_NORMAL){
    	if(kb_device_info.dongle_id == U32_MAX){ //link ERR deepback,change to SYNCING
    		kb_status->kb_mode = STATE_SYNCING;
    	}
    	else{  //link OK deepback
    		kb_status->dongle_id = kb_device_info.dongle_id;
    	}
    }
}

/*
 * Save the information need from the deep sleep back
 * kb_status: input parameter
 */
void kb_device_info_save(kb_status_t *kb_status)
{
    u8 * pd = (u8 *) &kb_device_info;
    //if watchdog trigger, this flag will increase by 2
#if PM_POWERON_DETECTION_ENABLE
    kb_device_info.mode = 0xe5;
#else
    kb_device_info.mode = kb_device_info.poweron - 2;
#endif
    kb_device_info.dongle_id = rf_get_access_code1();
    for (u8 i = PM_REG_START; i <= PM_REG_END; i++) {
        analog_write (i, *pd ++);
    }
}


