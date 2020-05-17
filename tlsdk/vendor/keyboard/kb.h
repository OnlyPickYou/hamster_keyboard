#pragma once
#include "../common/mouse_type.h"
#include "kb_default_config.h"
/////////////////// set default   ////////////////

#include "../common/default_config.h"
/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif


#ifndef MOUSE_DEEPSLEEP_EN
#define MOUSE_DEEPSLEEP_EN			1
#endif


#define SWS_DATA_OUT 			1   //sws pullup: output high, output disable


typedef enum{
	STATE_POWERON = 0,        
	STATE_SYNCING,
	STATE_PAIRING ,
	STATE_NORMAL,
	STATE_SUSPEND,
	STATE_DEEPSLEEP,
	STATE_EMI,
	STATE_WAIT_DEEP,
}MOUSE_MODE;


void mouse_task_when_rf ( void );
kb_status_t* kb_proc_get_kb_status(void);


/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

