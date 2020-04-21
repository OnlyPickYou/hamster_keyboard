/*
 * kb_device_info.h
 *
 *  Created on: Sep 1, 2014
 *      Author: qcmao
 */

#ifndef KB_DEVICE_INFO_H_
#define KB_DEVICE_INFO_H_

typedef struct{
	u32 dongle_id;
	u8	rsv;
    u8	channel;
	u8	mode;
	u8	poweron;
} kb_device_info_t;


extern void kb_device_info_load(kb_status_t *kb_status);

extern void kb_device_info_save(kb_status_t *kb_status);

#endif /* KB_DEVICE_INFO_H_ */
