/*
 * debug_paring.h
 *
 *  Created on: 2014-3-11
 *      Author: hp
 */

#ifndef DEBUG_PARING_H_
#define DEBUG_PARING_H_



#define DEBUG_PARING_SIHUI   1

#if(DEBUG_PARING_SIHUI)

	#define SIMULATE_DEVICE1  1

    #define PARING_ENABLE_ON_POWER   0


	#define BUTTON_LEFT          	0x01
	#define BUTTON_RIGHT         	0x02
	#define BUTTON_MIDDLE       	0x04

	#define PARING_MODE             (BUTTON_LEFT | BUTTON_RIGHT)

	#define	MOUSE_BUTTON_LEFT		GPIO_GP1
	#define	MOUSE_BUTTON_RIGHT		GPIO_GP2
	#define	MOUSE_BUTTON_MIDDLE		GPIO_GP3
	#define SET_GPIO_INPUT          reg_gpio_f_ie |= (GPIO_GP1 | GPIO_GP2 | GPIO_GP3);reg_gpio_f_oe &= ~(GPIO_GP1 | GPIO_GP2 | GPIO_GP3);reg_gpio_f_datao |= (GPIO_GP1 | GPIO_GP2 | GPIO_GP3)


	extern mouse_data_t     mouse_data;

	u8  button_last =0;
	u8  test_mode_paring_pending = 0;
	volatile u8  test_mode_paring = 0;

	u32 manual_paring_time;

	int proc_button(void)
	{
			*(u32 *) &mouse_data = 0;
			unsigned int button = 0;
			if (MOUSE_BUTTON_LEFT && !gpio_read(MOUSE_BUTTON_LEFT))
				button |= BUTTON_LEFT;
			if (MOUSE_BUTTON_RIGHT && !gpio_read(MOUSE_BUTTON_RIGHT)) {
				button |= BUTTON_RIGHT;
			}
			if (MOUSE_BUTTON_MIDDLE && !gpio_read(MOUSE_BUTTON_MIDDLE)) {
				button |= BUTTON_MIDDLE;
			}

			static u8 btn_power_on_mask = 0x0;


			if (!button)
				btn_power_on_mask = 0xff;
			mouse_data.btn = button & 7 & btn_power_on_mask;

			button_last = button;

			if(mouse_data.btn)
			{
				return 1;
			}

			return 0;
	}

	void proc_paring (void)
	{
		extern u8	button_last;
		static u8	button_pre = 0xff;
		static u8	power_on = 0;


		if(test_mode_paring && clock_time_exceed(manual_paring_time,3000000))  //手动配对时间  3s
		{
				test_mode_paring = 0;
		}

		#if (PARING_ENABLE_ON_POWER)
			static u32 tick_paring = 0;
			static int manual_paring = 0;
			if (!power_on)
			{
					test_mode_paring = PKT_FLOW_TOKEN | PKT_FLOW_PARING;
			}
			else if (!manual_paring && tick_paring >= 150)  //8 ms * 150 =1200 ms
			{
					test_mode_paring = 0;
			}
			tick_paring++;
		#endif

		int cmd = 0x80;
		if (button_pre != button_last)  //new event
		{
				int	button = button_last;
				if (!power_on)
				{
					button |= 0x80;
					power_on = 1;
				}

				if (button & 0x80)
				{
					if (button_last == PARING_MODE)
					{
						test_mode_paring_pending = PKT_FLOW_PARING;
						#if (PARING_ENABLE_ON_POWER)
							manual_paring = 1;
						#endif
					}
				}


				if (test_mode_paring_pending && !button_last)  //组合按键释放
				{
					manual_paring_time = clock_time();
					test_mode_paring = test_mode_paring_pending;
					test_mode_paring_pending = 0;
				}

		}
		button_pre = button_last;

	}



#endif



#endif /* DEBUG_PARING_H_ */
