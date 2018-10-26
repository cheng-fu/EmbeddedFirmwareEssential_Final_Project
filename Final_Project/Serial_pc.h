/*
 * Serial_pc.h
 *
 *  Created on: Oct 21, 2018
 *      Author: fu
 */

#ifndef SERIAL_PC_H_
#define SERIAL_PC_H_

#include "mbed.h"

Serial pc(USBTX,USBRX);

#ifndef unint
typedef unsigned int unint;
#endif

// Global timer act as a clock
Timer global_tm;

// Timeout used to control display off event
Timeout DisplayOff_to;
void reschedule_DisplayOff_TO(float TO);
const float default_DisplayOff_TO = 20;
volatile int display_state = 1;
volatile int device_face_down = 0;

#endif /* SERIAL_PC_H_ */
