/*
 * Accel_ADXL345_I2C.h
 *
 *  Created on: Oct 7, 2018
 *      Author: fu
 */

#ifndef ACCEL_ADXL345_I2C_H_
#define ACCEL_ADXL345_I2C_H_

#include "mbed.h"
#include "Serial_pc.h"
#include "SystemPowerManagement.h"

#define ACCEL_DEBUG_PRINT false

I2C accel(p9,p10);
static const int accel_addr = 0xA6; // accel ADXL345 default I2C address
const float accelSensitivity = 0.0039;	// G's/LSB (per ADXL345 Datasheet)

InterruptIn accel_INT1(p11);
DigitalOut red_led(p5);


class Accel_ADXL345 {
public:
	Accel_ADXL345();
	void stop();
	void start();
	void start_accel_event_detection();
	//void stop_accel_event_detection();
	void read_accel_data(int16_t * data_buffer);
protected:
	int init_sensor();
	int stop_sensor();
	void check_error(int error, const char * error_str);
	bool sensor_active = false;
};

Accel_ADXL345::Accel_ADXL345(){
	if (this->init_sensor() == 0) {
		this->sensor_active = true;
	}
}

void Accel_ADXL345::start(){
	if (!this->sensor_active) {
		if (this->init_sensor() == 0) {
			this->sensor_active = true;
		}
	}
}

void Accel_ADXL345::stop(){
	if (this->sensor_active) {
		if (this->stop_sensor() == 0) {
			this->sensor_active = false;
		}
	}
}

void Accel_ADXL345::check_error(int error, const char * error_str) {
	if (error != 0) {
		pc.printf("Error: %s\r\n", error_str);
	}
}

int Accel_ADXL345::init_sensor(){
	int err = 0;
	// Initialize ADXL345
	char config[2];
	config[0] = 0x31; // ADXL345 Data Format Register
	config[1] = 0x0B; // format +/-16g, 0.003g/LSB
	err = accel.write(accel_addr, config, 2);
	check_error(err, "Failed to set data format reg");
	config[0] = 0x2D; // ADXL345 Power Control Register
	config[1] = 0x08;// Select measure mode
	err = accel.write(accel_addr, config, 2);
	check_error(err, "Failed to set measure mode");
	return err;
}


int Accel_ADXL345::stop_sensor(){
	int err = 0;
	//char config[2];
	return err;
}

void Accel_ADXL345::read_accel_data(int16_t * data_buffer){
	char buffer[6];
    // Read 6-byte data packet into a buffer
	char base_addr = 0x32;
	int err = 0;
	err = accel.write(accel_addr, &base_addr, 1);
	check_error(err, "Failed to set base address");
	err = accel.read(accel_addr, buffer, 6);
	check_error(err, "Failed to read accel data");
	// Interpret the raw data bytes into meaningful results
	data_buffer[0] = buffer[1]<<8 | buffer[0]; 	// Combine MSB with LSB
	data_buffer[1] = buffer[3]<<8 | buffer[2];
	data_buffer[2] = buffer[5]<<8 | buffer[4];
}

// INT_ENABLE register settings
#define INT_ENABLE_addr 0x2E
#define INT_MAP_addr 0x2F
#define INT_SOURCE_addr 0x30
#define INT_EN_DATA_READY 0x80
#define INT_EN_SINGLE_TAP 0x40
#define INT_EN_ACTIVITY 0x10
// Activity threshold setting registers
#define THRESH_ACT_addr 0x24
#define ACT_INACT_CTL_addr 0x27
#define ACT_X_enable 0x40
#define ACT_Y_enable 0x20


void Accel_ADXL345::start_accel_event_detection() {
	/* Initialize accel shock detection interrupt */
	char config[2];
	int err = 0;
	config[0] = INT_ENABLE_addr;
	config[1] = INT_EN_ACTIVITY;
	err = accel.write(accel_addr, config, 2);
	check_error(err, "Failed to set accel shock detect interrupt");
	config[0] = INT_MAP_addr;
	config[1] = 0;
	err = accel.write(accel_addr, config, 2);
	check_error(err, "Failed to set accel shock detect interrupt map");
	config[0] = THRESH_ACT_addr;
	// set threshold to 20 MSB (~0.3g)
	config[1] = 30;
	err = accel.write(accel_addr, config, 2);
	check_error(err, "Failed to set accel activity threshold");
	config[0] = ACT_INACT_CTL_addr;
	// set x y axis involved
	config[1] = ACT_X_enable | ACT_Y_enable;
	err = accel.write(accel_addr, config, 2);
	check_error(err, "Failed to set accel activity control register");
#if ACCEL_DEBUG_PRINT
	pc.printf("accel_shock_detect init done\r\n");
#endif
}

/*
 * Accel interrupt management
 * */

uint16_t accel_int_cnt = 0;
uint16_t int_src = 0;
Timeout accel_ISR_reenable_TO;
const float accel_ISR_disable_interval = 0.075;

void accel_shock_ISR();

char read_INT_SOURCE() {
	// read interrupt event source on accel ADXL345 clears out interrupt
	// Runs in interrupt context
	char addr = INT_SOURCE_addr;
	char buffer[1];
	int err = 0;
	err = accel.write(accel_addr, &addr, 1);
	err = accel.read(accel_addr, buffer, 1);
	if (err != 0)
		return 0;

	return buffer[0];
}

void accel_ISR_reenable_Callback(){
	read_INT_SOURCE();
	accel_INT1.rise(&accel_shock_ISR);
}

void accel_shock_ISR() {
	accel_int_cnt += 1;
	red_led = ! red_led;
	if (!device_face_down || is_system_asleep()) {
	    reschedule_DisplayOff_TO(default_DisplayOff_TO);
	}
	accel_INT1.rise(NULL);
	int_src = read_INT_SOURCE();
	// Schedule re-enable ISR Timeout call back
	accel_ISR_reenable_TO.attach(&accel_ISR_reenable_Callback, accel_ISR_disable_interval);
}

void disable_IRQ(){
	accel_ISR_reenable_TO.detach();
	accel_INT1.rise(NULL);
	red_led = 0;
};

void enable_IRQ() {
	accel_int_cnt = 0;
	red_led = 0;
#if ACCEL_DEBUG_PRINT
	pc.printf("accel_shock_detect int enabled\r\n");
#endif
	accel_INT1.rise(&accel_shock_ISR);
}

#endif /* ACCEL_ADXL345_SPI_H_ */
