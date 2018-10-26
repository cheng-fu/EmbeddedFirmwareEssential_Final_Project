/*
 * Temp102.h
 *
 *  Created on: Oct 7, 2018
 *      Author: fu
 */

#ifndef TEMP102_H_
#define TEMP102_H_

#include "mbed.h"

const float TempSensitivity = 0.0625;	// degrees C/LSB (per TMP102 Data sheet)
static const int addr = 0x90;	// Default I2C address of TMP102


I2C tempsensor(p9,p10);

char config_t[3];


void init_Temp102() {
	// Initialize TMP102
	config_t[0] = 0x01;		// set pointer register to 'config register' (Table 7 data sheet)
	config_t[1] = 0x60;		// config temperature measurements to 12-bit resolution (Table 10 data sheet)
	config_t[2] = 0xA0;		// configure temperature conversion rate to 4 Hz, AL to normal (Table 11 data sheet)
	tempsensor.write(addr, config_t, 3);	// write 3 bytes to device at address 0x90
}

void start_Temp102() {
	// Start temperature measurments at a 4Hz rate
	config_t[0] = 0x00;		// set pointer register to 'data register' (Table 7 datasheet)
	tempsensor.write(addr, config_t,1);		// send to pointer 'read temp'
}

float read_Temp102() {
	char temp_read[2];
	float temp;
	tempsensor.read(addr, temp_read, 2);	// read the 2-byte temperature data
	// convert to 12-bit temp data (see Tables 8 & 9 in data sheet)
	temp = TempSensitivity * (((temp_read[0] << 8) + temp_read[1]) >> 4);
	return temp;
}

#endif /* TEMP102_H_ */
