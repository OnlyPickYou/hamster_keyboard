/*
 * kb_batt.h
 *
 *  Created on: Sep 1, 2014
 *      Author: qcmao
 */

#ifndef KB_BATT_H_
#define KB_BATT_H_

extern int kb_batt_det_flg;

void kb_batt_det_init(void);
int kb_batt_det_process(void);

#endif /* KB_BATT_H_ */
