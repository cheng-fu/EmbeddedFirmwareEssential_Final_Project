/*
 * Display_Brightness_control.h
 *
 *  Created on: Oct 21, 2018
 *      Author: fu
 */

#ifndef DISPLAY_BRIGHTNESS_CONTROL_H_
#define DISPLAY_BRIGHTNESS_CONTROL_H_

#include "mbed.h"
#include "Serial_pc.h"

#define Display_Brightness_LOG_ENABLE true

PwmOut Display_Backlight_ctl(p25);

static const unsigned int pwm_period_us = 2000;

static const float defualt_TO = 0.04;
static volatile float step = 0.0;
static const unint step_cnt = 8;
static volatile unint cur_step = 0;

// Display timeout to control brightness change smoothing
Timeout display_timeout;

void schedule_TO(float TO);

void adjust_brightness_smooth() {
	Display_Backlight_ctl = Display_Backlight_ctl + step;
	schedule_TO(defualt_TO);
}

void schedule_TO(float TO) {
	if (cur_step > 0) {
		display_timeout.attach(&adjust_brightness_smooth, defualt_TO);
		cur_step -= 1;
	}
}
class Display_Brightness_Controller {
public:
	Display_Brightness_Controller();
	void set_brightness(float val);
    void adjust_brightness(unint ALS_ch0_val);
    float get_brightness();
    void turn_off_display();
    void turn_on_display();
protected:
    float last_brightness = 0.0;
};

Display_Brightness_Controller::Display_Brightness_Controller() {
	Display_Backlight_ctl.period_us(pwm_period_us);
	this->set_brightness(0.5);
}

void Display_Brightness_Controller::set_brightness(float val) {
	if (val <= 0.0) {
		val = 0.0;
	} else if (val >= 1.0) {
		val = 1.0;
	}
	// If a smoothed adjust in progress, ignore set brightness request
	if (cur_step > 0) return;

	float cur_brightness = Display_Backlight_ctl;
	if (val - cur_brightness > 0.03) {
		// If target value tunes up over 0.03, smooth it with step_cnt times change
		step = 1.0 * (val - cur_brightness) / step_cnt;
		cur_step = step_cnt;
		schedule_TO(defualt_TO);
	} else if (val - cur_brightness < -0.03) {
		// If target value tunes down over 0.03, smooth it with 3 * step_cnt times change
		step = 1.0 * (val - cur_brightness) / (step_cnt * 2);
		cur_step = step_cnt * 2;
		schedule_TO(defualt_TO);
	}else {
		Display_Backlight_ctl = val;
	}
	last_brightness = val;
}

float Display_Brightness_Controller::get_brightness() {
	return Display_Backlight_ctl;
}

void Display_Brightness_Controller::adjust_brightness(unint ALS_ch0_val) {
	unint min = 15;
	unint med = 300;
	unint max = 1500;
	float new_brightness = 0.0;
	if (ALS_ch0_val == 0) return;
	if (ALS_ch0_val <= min) ALS_ch0_val = min;
	if (ALS_ch0_val < med)
		new_brightness = 0.4 * (ALS_ch0_val - min) / (med - min) + 0.1;
	else
		new_brightness = 0.5 * (ALS_ch0_val - med) / (max - med) + 0.5;
	this->set_brightness(new_brightness);
}

void Display_Brightness_Controller::turn_off_display() {
    while(cur_step > 0) {
    	display_timeout.detach();
    	cur_step = 0;
    }
    Display_Backlight_ctl = 0;
}

void Display_Brightness_Controller::turn_on_display() {
    while(cur_step > 0) {
        display_timeout.detach();
        cur_step = 0;
    }
    Display_Backlight_ctl = last_brightness;
}

#endif /* DISPLAY_BRIGHTNESS_CONTROL_H_ */
