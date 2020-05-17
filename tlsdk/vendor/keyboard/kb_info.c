#include "../../proj/tl_common.h"
#include "../../proj/mcu/watchdog_i.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../common/rf_frame.h"
#include "kb_info.h"
#include "kb.h"
#include "kb_rf.h"

void kb_info_load(void)
{
    kb_status_t *kb_status = NULL;
    kb_status = kb_proc_get_kb_status();
    assert(NULL != kb_status);

	kb_status->mode_link = analog_read(PM_REG_MODE_LINK);
	u8 check_read = analog_read(PM_REG_CHECK);

	u8 check_cal = 0;
	u8 * pd = (u8 *) (&kb_status->dongle_id);
	for (u8 i = PM_REG_DONGLE_ID_START; i <= PM_REG_DONGLE_ID_END; i++) {
		*pd  = analog_read (i);
		check_cal += *pd;
		pd++;
	}

	if((kb_status->mode_link&LINK_PIPE_CODE_OK) && check_cal==check_read && kb_status->dongle_id!=U32_MAX){
		kb_status->kb_mode = STATE_NORMAL;
	}else{
		kb_status->mode_link = 0;
		kb_status->dongle_id = 0;
	}

}

void kb_info_save(void)
{

    kb_status_t *kb_status = NULL;
    kb_status = kb_proc_get_kb_status();
    assert(NULL != kb_status);

	u8 * pd;
	u8 check = 0;

	kb_status->dongle_id = rf_get_access_code1();
	pd = (u8 *) &kb_status->dongle_id;
	for (u8 i = PM_REG_DONGLE_ID_START; i <= PM_REG_DONGLE_ID_END; i++) {
		check += *pd;
		analog_write (i, *pd ++);
	}

	analog_write(PM_REG_MODE_LINK,kb_status->mode_link&LINK_PIPE_CODE_OK);
	analog_write(PM_REG_CHECK,check);

}
