/*
 * Temp102.h
 *
 *  Created on: Oct 7, 2018
 *      Author: fu
 */

#ifndef TEMP102_H_
#define TEMP102_H_

#include "mbed.h"
#include "Serial_pc.h"


I2C tempsensor(p9,p10);

class TMP_102 {
public:
	TMP_102();
	void stop();
	void start();
	float read_data();
	static constexpr float TempSensitivity = 0.0625;	// degrees C/LSB (per TMP102 Data sheet)
	static const int i2c_addr = 0x90;	// Default I2C address of TMP102
protected:
	int init_sensor();
	void check_error(int error, const char * error_str);
	bool sensor_active = false;
	char config_t[3];
};

TMP_102::TMP_102(){
	if (this->init_sensor() == 0) {
		this->sensor_active = true;
	}
}

int TMP_102::init_sensor(){
	int err = 0;
	// Initialize TMP102
	config_t[0] = 0x01;		// Set pointer register to 'config register' (Table 7 data sheet)
	config_t[1] = 0x60;		// configure temperature measurements to 12-bit resolution (Table 10 data sheet)
	config_t[2] = 0xA0;		// configure temperature conversion rate to 4 Hz, AL to normal (Table 11 data sheet)
	err = tempsensor.write(this->i2c_addr, config_t, 3);	// write 3 bytes to device at address 0x90
	this->check_error(err, "Failed to set TMP config reg");
	return err;
}


void TMP_102::check_error(int error, const char * error_str) {
	if (error != 0) {
		pc.printf("Error: %s\r\n", error_str);
	}
}

void TMP_102::start(){
	if (not this->sensor_active) {
		if (this->init_sensor() == 0) {
			this->sensor_active = true;
		}
	}
	// Start temperature measurments at a 4Hz rate
	config_t[0] = 0x00;		// set pointer register to 'data register' (Table 7 datasheet)
	int err = tempsensor.write(this->i2c_addr, config_t,1);		// send to pointer 'read temp'
	this->check_error(err, "Failed to set pointer register to data register");
}

void TMP_102::stop(){
	// stop temperature measurement
	if (not this->sensor_active) return;
	// Enable Shutdown mode on TMP102
	config_t[0] = 0x01;		// Set pointer register to 'config register' (Table 7 data sheet)
	config_t[1] = 0x61;		// configure temperature measurements to shutdown mode, set bit 0 (SD) =1
	config_t[2] = 0xA0;
	int err = tempsensor.write(this->i2c_addr, config_t, 3);	// write 3 bytes to device at address 0x90
	if (err == 0) this->sensor_active = false;
	this->check_error(err, "Failed to set TMP to shutdown mode");
}

float TMP_102::read_data() {
	char temp_read[2];
	float temp;
	tempsensor.read(this->i2c_addr, temp_read, 2);	// read the 2-byte temperature data
	// convert to 12-bit temp data (see Tables 8 & 9 in data sheet)
	temp = this->TempSensitivity * (((temp_read[0] << 8) + temp_read[1]) >> 4);
	return temp;
}

#endif /* TEMP102_H_ */
