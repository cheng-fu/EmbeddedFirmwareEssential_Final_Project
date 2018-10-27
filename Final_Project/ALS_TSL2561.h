/*
 * ALS_TSL2561.h
 *
 *  Created on: Oct 21, 2018
 *      Author: fu
 */

#ifndef ALS_TSL2561_H_
#define ALS_TSL2561_H_


#define ALS_DEBUG_PRINT false
#include "mbed.h"
#include "Serial_pc.h"

static const int als_addr = 0x39 << 1;
static I2C als_i2c(p9, p10);

class ALS_TSL2561 {
public:
	ALS_TSL2561();
	void stop();
	void start();
	int read_data(unint * buffer);
protected:
	int init_sensor();
	int stop_sensor();
	void check_error(int error, const char * error_str);
	bool sensor_active = false;
};

// Register Address Map
#define TSL2561_CONTROL_REG 0x00
#define TSL2561_DATA0_LOW 0x0C
#define TSL2561_DATA1_LOW 0x0E
#define CMD_BLOCK_RW 0xA0

ALS_TSL2561::ALS_TSL2561() {
	if (this->init_sensor() == 0) {
		this->sensor_active = true;
	}
}

void ALS_TSL2561::check_error(int error, const char * error_str) {
	if (error != 0) {
		pc.printf("Error: %s\r\n", error_str);
	}
}

void ALS_TSL2561::start(){
	if (!this->sensor_active) {
		if (this->init_sensor() == 0) {
			this->sensor_active = true;
		}
	}
}

void ALS_TSL2561::stop(){
	if (this->sensor_active) {
		if (this->stop_sensor() == 0) {
			this->sensor_active = false;
		}
	}
}

int ALS_TSL2561::stop_sensor(){
	char config[2];
	int err = 0;
	// power down the sensor
	config[0] = CMD_BLOCK_RW | TSL2561_CONTROL_REG;
	config[1] = 0x00;
	err = als_i2c.write(als_addr, config, 2);
	this->check_error(err, "Failed to set TSL2561 Control reg");
	// At this point both ADC channels will begin a conversion at the default integration time of 400ms
	// After 400ms the conversion results will be available in DATA0 and DATA1 registers
	return err;
}

int ALS_TSL2561::init_sensor(){
	char config[2];
	int err = 0;
	// power up the sensor
	config[0] = CMD_BLOCK_RW | TSL2561_CONTROL_REG;
	config[1] = 0x03;
	err = als_i2c.write(als_addr, config, 2);
	this->check_error(err, "Failed to set TSL2561 Control reg");
	// At this point both ADC channels will begin a conversion at the default integration time of 400ms
	// After 400ms the conversion results will be available in DATA0 and DATA1 registers
	return err;
}

int ALS_TSL2561::read_data(unint * buffer){
	int err = 0;
	char raw_data[4];
	char data_low_addr = TSL2561_DATA0_LOW | CMD_BLOCK_RW;
	err = als_i2c.write(als_addr, &data_low_addr, 1);
	this->check_error(err, "Failed to set TSL2561 read addr to DATA0_LOW");
	err = als_i2c.read(als_addr, raw_data, 2);
	this->check_error(err, "Failed to read TSL2561 Raw Data 1");
	data_low_addr = TSL2561_DATA1_LOW | CMD_BLOCK_RW;
	err = als_i2c.write(als_addr, &data_low_addr, 1);
	this->check_error(err, "Failed to set TSL2561 read addr to DATA1_LOW");
	err = als_i2c.read(als_addr, &raw_data[2], 2);
	this->check_error(err, "Failed to read TSL2561 Raw Data 1");
	buffer[0] = unint(unint(raw_data[1]) << 8) + raw_data[0];
	buffer[1] = unint(unint(raw_data[3]) << 8) + raw_data[2];
#if ALS_DEBUG_PRINT
	pc.printf("als Data 0: %d\r\n", raw_data[0]);
	pc.printf("als Data 1: %d\r\n", raw_data[1]);
	pc.printf("als Data 2: %d\r\n", raw_data[2]);
	pc.printf("als Data 3: %d\r\n", raw_data[3]);
#endif
	return err;
}

#endif /* ALS_TSL2561_H_ */
