
#pragma once
#if (__PROJECT_PM_TEST_5330__)	//  min environment for testing PM functionality.
	#define __PROJECT_PM_TEST__ 			1
	#include "example_8566.h"
#elif (__PROJECT_KEYBOARD_8366__)
    #include "../keyboard/kb.h"
	#include "../keyboard/kb_default_config.h"
#elif (__PROJECT_KEYBOARD__)
	#include "../keyboard/kb_main.h"
    #include "../keyboard/kb_default_config.h"
#elif (__PROJECT_DONGLE__)
	#include "../dongle/dongle.h"
#elif (__PROJECT_KM_SNIFFER__)
	#include "../km_sniffer/km_sniffer.h"
#elif (__PROJECT_PKT_SNIFFER__)
	#include "../pktsniffer/main_pktsniffer.h"
#elif (__PROJECT_DONGLE_REMOTE__)
	#include "../dongle_remote/app_config.h"
#elif (__PROJECT_REMINGTON_KEYBOARD__)
	#include "../keyboard_remington/kb_default_config.h"
	#include "../keyboard_remington/kb.h";
#else
	#include "user_config_common.h"
#endif
