/*
 * kb_ota.h
 *
 *  Created on: 2014-11-18
 *      Author: Telink
 */

#ifndef KB_OTA_H_
#define KB_OTA_H_

#define KB_KEY_XYZ   (kb_event.cnt == 3 && kb_event.keycode[0] < 0x1e && kb_event.keycode[0] > 0x1a \
										&& kb_event.keycode[1] < 0x1e && kb_event.keycode[1] > 0x1a \
										&& kb_event.keycode[2] < 0x1e && kb_event.keycode[2] > 0x1a )

#define KB_KEY_OPQ   (kb_event.cnt == 3 && kb_event.keycode[0] < 0x15 && kb_event.keycode[0] > 0x11 \
										&& kb_event.keycode[1] < 0x15 && kb_event.keycode[1] > 0x11 \
										&& kb_event.keycode[2] < 0x15 && kb_event.keycode[2] > 0x11 )

#define KB_KEY_MN    (kb_event.cnt == 2 && kb_event.keycode[0] < 0x12 && kb_event.keycode[0] > 0x0f \
										&& kb_event.keycode[1] < 0x12 && kb_event.keycode[1] > 0x0f )


#define KB_OTA_ST_NONE		   0x00
#define KB_OTA_ST_CMD_HALF     0x01
#define KB_OTA_ST_BEGIN        0x02


void kb_ota_init(void);
void kb_ota_mode_detect(kb_status_t *kb_status);
void kb_ota_process(kb_status_t *kb_status);

#endif /* KB_OTA_H_ */
