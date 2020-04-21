#pragma once
/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include "..\..\proj\common\types.h"
#include "../common/device_led.h"
#include "../../proj/drivers/keyboard.h"

#ifndef CLOCK_SYS_CLOCK_HZ
#define CLOCK_SYS_CLOCK_HZ  	16000000
#endif

#define _USER_CONFIG_DEFINED_	1	// must define this macro to make others known 

typedef struct {
	u32 button[8+18];
	u32 led_cntr;
	u32 gpio_level_button;
	u32 vbat_channel;
//	u32 wheel[2];
//	u32 sensor_data;
//	u32 sensor_sclk;
//	u32 sensor_int;
}kb_hw_t;

typedef struct {

	u8  kb_mode;                //EMI mode, power on, pairing auto/manual, normal mode etc
	u8  dbg_mode;
	u8  high_end;                  //high-end mouse or low-end
	u8  no_ack;                    //indicate whether get the ACK for previous package send

	u8  mouse_sensor;
	u8  cpi;
	u8  sensor_dir;
	s8  wheel_dir;

	u32 dongle_id;
	u32 device_id;

    u8  rf_mode;					//rf data /pairing / idle, also make mouse_data_t aligned to 4
	u8  tx_power;
	u8  tx_retry;
    u8  rcv_rssi;

    u32 pkt_addr;
    u32 loop_cnt;

	kb_hw_t			*hw_define;
	kb_data_t		*data;
    device_led_t	*led_define;

} kb_status_t;

typedef enum{
	STATE_POWERON = 0,	//0
	STATE_SYNCING,
	STATE_PAIRING,
	STATE_NORMAL,
	STATE_EMI,
	STATE_OTA,

}KB_MODE;

#define STATE_TEST_0_BIT    0x80
#define STATE_TEST_EMI_BIT  0x40
#define STATE_TEST_V_BIT    0x20

#define  KB_WK_198          0

#if(KB_WK_198)

#define  KB_DRIVE_PINS  {GPIO_PC5, GPIO_PC6, GPIO_PC4, GPIO_PA5, GPIO_PD0, GPIO_PD1, GPIO_PC2, GPIO_PB6}
#define  KB_SCAN_PINS  {GPIO_PE4, GPIO_PA7, GPIO_PB7,  GPIO_PD7, GPIO_PE1, GPIO_PE2, \
						GPIO_PE5, GPIO_PE6, GPIO_PF0,  GPIO_PC1, GPIO_PF1, GPIO_PC0, \
						0, 		  0, 		GPIO_PC3,  GPIO_PE7, GPIO_PA6, 0 }

#define	MATRIX_ROW_PULL		PM_PIN_PULLUP_1M
#define	MATRIX_COL_PULL		PM_PIN_PULLDOWN_100K

//drive pin 8
#define	PULL_WAKEUP_SRC_PC5		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC6		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC4		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PA5		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD0		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD1		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC2		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PB6		MATRIX_ROW_PULL
//scan pin 18
#define	PULL_WAKEUP_SRC_PE4		MATRIX_COL_PULL  //C0
#define	PULL_WAKEUP_SRC_PA7		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB7		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PD7		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PE1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PE2		MATRIX_COL_PULL  //C5
#define	PULL_WAKEUP_SRC_PE5		MATRIX_COL_PULL  //C6
#define	PULL_WAKEUP_SRC_PE6		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PF0		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PC1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PF1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PC0		MATRIX_COL_PULL //C11
                                                //C12
											    //C13
#define	PULL_WAKEUP_SRC_PC3		MATRIX_COL_PULL //C14
#define	PULL_WAKEUP_SRC_PE7		MATRIX_COL_PULL //C15
#define	PULL_WAKEUP_SRC_PA6		MATRIX_COL_PULL //C16

#define	PA7_FUNC				AS_GPIO
#define	PB5_FUNC				AS_GPIO
#define	PB6_FUNC				AS_GPIO

#define GPIO_LED_IND            GPIO_PD3
#define GPIO_LED_CAP          	GPIO_PD4
#define GPIO_LED_SCR			GPIO_PD6

#define GPIO_BAT_DET  			GPIO_PD2
#define ADC_CHN_BAT_DET			FLD_ADC_CHN_D2

#else


#define  KB_DRIVE_PINS  {GPIO_PB7, GPIO_PC0, GPIO_PC1, GPIO_PC2, GPIO_PC3, GPIO_PC4, GPIO_PC5, GPIO_PC6}
#define  KB_SCAN_PINS  {GPIO_PB1, GPIO_PD7, GPIO_PE1,  GPIO_PE2, GPIO_PE4, GPIO_PE5, \
						GPIO_PE6, GPIO_PE7, GPIO_PA7,  GPIO_PB0, GPIO_PA6, GPIO_PA4, \
						GPIO_PA1, GPIO_PF1, GPIO_PF0,  GPIO_PA5, GPIO_PB5, GPIO_PB6 }

#define	MATRIX_ROW_PULL		PM_PIN_PULLUP_1M
#define	MATRIX_COL_PULL		PM_PIN_PULLDOWN_100K

//drive pin 8
#define	PULL_WAKEUP_SRC_PB7		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC0		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC1		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC2		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC3		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC4		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC5		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC6		MATRIX_ROW_PULL
//scan pin 18
#define	PULL_WAKEUP_SRC_PB1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PD7		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PE1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PE2		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PE4		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PE5		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PE6		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PE7		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA7		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB0		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA6		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA4		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PF1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PF0		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA5		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB5		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB6		MATRIX_COL_PULL

#define	PA7_FUNC				AS_GPIO
#define	PB5_FUNC				AS_GPIO
#define	PB6_FUNC				AS_GPIO

#define GPIO_LED_IND            GPIO_PC7

#define GPIO_BAT_DET  			GPIO_PD3
#define ADC_CHN_BAT_DET			FLD_ADC_CHN_D3

#endif
/////////////////// set default   ////////////////

#include "../common/default_config.h"

/////////////////// main loop, event loop  ////////////////
enum {
	EV_FIRED_EVENT_MAX = 8
};

typedef enum {
	EV_SUSPEND_NOTIFY,
	EV_WAKEUP_NOTIFY,
	EV_KEY_PRESS,
#if(MOUSE_USE_RAW_DATA)
	EV_MOUSE_RAW_DATA,
#endif	
	EV_RF_PKT_RECV,
	EV_PAIRING_START,
	EV_PAIRING_STOP,
	EV_MOUSE_EVENT,
	EV_KEYBOARD_EVENT,
#if(MODULE_SOMATIC_ENABLE)	
	EV_SOMATIC_EVENT,
#endif
	EV_EVENT_MAX,
} ev_event_e;

typedef enum {
	EV_POLL_MOUSE_EVENT, EV_POLL_KEYBOARD_EVENT,
#if(MODULE_SOMATIC_ENABLE)	
	EV_POLL_SOMATIC_EVENT,
#endif	
	EV_POLL_RF_RECV, EV_POLL_DEVICE_PKT, EV_POLL_RF_CHN_HOPPING, EV_POLL_IDLE, //  Must be the last item in ev_poll_e
	EV_POLL_MAX,
} ev_poll_e;

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

