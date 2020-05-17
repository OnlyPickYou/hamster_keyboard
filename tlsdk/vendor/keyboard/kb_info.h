#ifndef _DEVICE_INFO_H_
#define _DEVICE_INFO_H_


#ifndef DEVICE_INFO_STORE
#define DEVICE_INFO_STORE   1
#endif

#include "../keyboard/kb_default_config.h"

#define		PM_REG_MODE_LINK		0x19
#define		PM_REG_CHECK		    0x1a
#define		PM_REG_LOCK		    	0x1b

#define		PM_REG_DONGLE_ID_START		0x1c
#define		PM_REG_DONGLE_ID_END		0x1f

typedef struct{
	u32 dongle_id;
    u8	channel;
	u8	mode;
	u8	poweron;
} kb_info_t;

void kb_info_load(void);
void kb_info_save(void);

#endif
