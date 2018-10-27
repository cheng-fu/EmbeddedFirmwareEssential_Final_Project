/*
 * SystemPower_Management.h
 *
 *  Created on: Oct 26, 2018
 *      Author: fu
 */

#ifndef SYSTEMPOWERMANAGEMENT_H_
#define SYSTEMPOWERMANAGEMENT_H_

#include <stdint.h>
#include "mbed_sleep.h"
#include "Serial_pc.h"

#define SYSTEM_CONTROL_REG (*(volatile uint32_t *) (0xE000ED10))
#define INTTERUPT_SET_ENABLE_REG (* (volatile uint32_t *) (0xE000E100))
#define INTTERUPT_SET_ENABLE_REG_STOP (* (volatile uint32_t *) (0xE000E11C))
#define INTTERUPT_CLEAR_ENABLE_REG (* (volatile uint32_t *) (0xE000E180))
#define INTTERUPT_CLEAR_ENABLE_REG_STOP (* (volatile uint32_t *) (0xE000E18C))

#define SYSTEM_SLEEP_DEBUG_PRINT true

volatile uint32_t sleep_cnt;
volatile int is_asleep = 0;
volatile unsigned int last_sleep_duration = 0;
volatile unsigned long total_sleep_time = 0;

// System power indicator
DigitalOut power_led(p28);

void system_sleep(){

	is_asleep = 1;
	power_led = 0;
	// Set SLEEPONEXIT bit, allow system power gating when finish ISR execution
	SYSTEM_CONTROL_REG |= 0x0002;
	int sleep_starts_at = global_tm.read_ms();
	// Put system to sleep
	sleep();
	sleep_cnt += 1;
	last_sleep_duration = global_tm.read_ms() - sleep_starts_at;
	total_sleep_time += last_sleep_duration;
};

bool is_system_asleep() {
	return is_asleep == 1;
};

void system_wakeup() {
	// Exit from sleep mode, runs in interrupt context
	// Clear SLEEPONEXIT bit
	SYSTEM_CONTROL_REG &= (~0x0002);
	is_asleep = 0;
	power_led = 1;
}



#endif /* SYSTEMPOWERMANAGEMENT_H_ */
