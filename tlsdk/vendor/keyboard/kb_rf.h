/*
 * kb_rf.h
 *
 *  Created on: 2015-1-21
 *      Author: Administrator
 */

#ifndef KB_RF_H_
#define KB_RF_H_



#define LINK_PIPE_CODE_OK  		0x01
#define LINK_RCV_DONGLE_DATA  	0x02
#define LINK_WITH_DONGLE_OK	    (LINK_PIPE_CODE_OK | LINK_RCV_DONGLE_DATA)

#define device_never_linked (rf_get_access_code1() == U32_MAX)

extern u8* kb_rf_pkt;
extern rf_packet_pairing_t	pkt_pairing;
extern rf_packet_keyboard_t	pkt_km;

extern int	km_dat_sending;

extern int kb_pairing_mode_detect(void);
extern void kb_paring_and_syncing_proc(void);

extern void kb_rf_proc(u32 key_scaned);
extern void kb_rf_init(void);

void irq_device_rx(void);
void irq_device_tx(void);


#define DO_TASK_WHEN_RF_EN      1
//typedef void (*callback_rx_func) (u8 *);
//typedef void (*task_when_rf_func) (void);

#endif /* KB_RF_H_ */
