#ifndef _DEVICE_INFO_H_
#define _DEVICE_INFO_H_


#ifndef DEVICE_INFO_STORE
#define DEVICE_INFO_STORE   1
#endif

#include "../mouse/kb_default_config.h"


typedef struct{
	u32 dongle_id;
	u8	sensor;
    u8	channel;
	u8	mode;
	u8	poweron;
} device_info_t;

#define INFO_SENSOR_CPI_CTRL    0x0f
#define INFO_SENSOR_STATUS_CTRL 0xf0

void device_info_load(kb_status_t *kb_status);
#if DEVICE_INFO_STORE
void device_info_save(kb_status_t *kb_status, u32 sleep_save);
#else
static inline void device_info_save(mouse_status_t *mouse_status, u32 sleep_save) {}
#endif

#endif
