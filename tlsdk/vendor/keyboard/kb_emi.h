/*
 * kb_emi.h
 *
 *  Created on: Sep 1, 2014
 *      Author: qcmao
 */

#ifndef KB_EMI_H_
#define KB_EMI_H_

#define 		PARING_KEY_MAX 		2

typedef struct{
	u8  emi_start;

	u8	emi_lf_carry;
	u8	emi_lf_cd;
	u8	emi_lf_rx;
	u8	emi_lf_tx;

	u8	emi_mf_carry;
	u8	emi_mf_cd;
	u8	emi_mf_rx;
	u8	emi_mf_tx;

	u8	emi_hf_carry;
	u8	emi_hf_cd;
	u8	emi_hf_rx;
	u8	emi_hf_tx;

	u8	rsv[3];
}kb_emi_info_t;



extern u32 kb_button_process_emi(s8 *chn_idx, u8 *test_mode_sel, u32 btn_pro_end);
extern void kb_emi_process(kb_status_t *kb_status);

#endif /* KB_EMI_H_ */
