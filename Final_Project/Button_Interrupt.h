/*
 * Button_Interrupt.h
 *
 *  Created on: Oct 24, 2018
 *      Author: fu
 */

#ifndef BUTTON_INTERRUPT_H_
#define BUTTON_INTERRUPT_H_

#include "mbed.h"
#include "Serial_pc.h"

#define BUTTON_DEBUG_PRINT false

#ifndef NULL
#define NULL (0)
#endif

InterruptIn Button_INT(p30);
DigitalOut green_led(p29);
Timeout button_tm;

volatile uint16_t but_int_cnt = 0;
const float button_TO = 0.025;

class Button {
public:
	Button();
	static void enable_IRQ();
	static void disable_IRQ();
};

void button_ISR() {
	but_int_cnt += 1;
	green_led = !green_led;
	Button::disable_IRQ();
	if (!device_face_down) {
	    reschedule_DisplayOff_TO(default_DisplayOff_TO);
	}
	button_tm.attach(&Button::enable_IRQ, button_TO);
}

Button::Button(){
	// initialize IRQ
	but_int_cnt = 0;
	green_led = 0;
	enable_IRQ();
#if BUTTON_DEBUG_PRINT
		pc.printf("Button int enabled\r\n");
#endif
}

void Button::enable_IRQ(){
	Button_INT.rise(&button_ISR);
	Button_INT.fall(&button_ISR);
}

 void Button::disable_IRQ(){
#if BUTTON_DEBUG_PRINT
		pc.printf("Button int disabled\r\n");
#endif
	Button_INT.rise(NULL);
	Button_INT.fall(NULL);
}

#endif /* BUTTON_INTERRUPT_H_ */
