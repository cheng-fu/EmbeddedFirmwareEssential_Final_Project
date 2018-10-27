/* Embedded Firmware Essentials - Fall 2018
 * Final Project
 * Created by Cheng Fu on 10/21/2018
 * Uses Simon Ford's TextLCD Library
*/

#include "mbed.h"

#include "Temp102.h"
#include "TextLCD.h"
#include "Accel_ADXL345_I2C.h"
#include "ALS_TSL2561.h"
#include "Serial_pc.h"
#include "Display_Brightness_control.h"
#include "Button_Interrupt.h"
#include "SystemPowerManagement.h"

TextLCD lcd(p19, p20, p24, p23, p22, p21); // rs, e, d4-d7

#define MAIN_LOG_ENABLE true

// SET to true if print on every loop, false every 10 loops
#define MAIN_VERBOSe_LOG false

ALS_TSL2561 als_sensor;
Accel_ADXL345 accel_sensor;
TMP_102 temp_sensor;
Display_Brightness_Controller display_brightness;
Button button;

int16_t accel_data[3];
float temp = 0.0;
float x, y, z;
volatile unint idle_loop_count = 0;

void DisplayOff_Callback(){
	if(display_state == 1) {
		als_sensor.stop();
		temp_sensor.stop();
	}
	display_state = 0;
	display_brightness.turn_off_display();
}

void reschedule_DisplayOff_TO(float TO) {
  if(is_system_asleep()) {
	  system_wakeup();
	  device_was_facing_down = 0;
  }
  DisplayOff_to.detach();
  if(display_state == 0) {
	  display_brightness.turn_on_display();
	  als_sensor.start();
	  temp_sensor.start();
	  idle_loop_count = 0;
  }
  display_state = 1;
  DisplayOff_to.attach(&DisplayOff_Callback, TO);
}


int main()
{
	unint als_data[2] = {0,0};
	pc.baud(115200);
	// initialize system power LED to 1
	power_led = 1;
	pc.printf("\r\n=====================\r\n");
	pc.printf("System starting...\r\n");
	global_tm.start();
	temp_sensor.start();
	accel_sensor.start_accel_event_detection();
	enable_IRQ();
	DisplayOff_to.attach(&DisplayOff_Callback, default_DisplayOff_TO);
	while(1)
	{
		if (idle_loop_count >= 62) {
#if MAIN_LOG_ENABLE
			pc.printf("Device going to sleep ... \r\n");
#endif
			system_sleep();
#if MAIN_LOG_ENABLE
			pc.printf("Device waken up from sleep ... \r\n");
			wait_ms(100);
#endif
			continue;
		}

		wait_ms(200);
		// Read Accel data
		accel_sensor.read_accel_data(accel_data);
		x = accel_data[0] * accelSensitivity;	// x-axis acceleration in G's
		y = accel_data[1] * accelSensitivity;	// y-axis acceleration in G's
		z = accel_data[2] * accelSensitivity;	// z-axis acceleration in G's

		// Figure out device orientation
		if (z < - 0.75) {
			device_face_down = 1;
			device_was_facing_down = 1;
			DisplayOff_Callback();
		} else {
			if (device_was_facing_down == 1 && display_state == 0 && z > 0.5) {
				reschedule_DisplayOff_TO(default_DisplayOff_TO);
				device_was_facing_down = 0;
			}
			device_face_down = 0;
		}
		bool new_samples_read = false;
		// Check display state and print to Text LCD
		if (display_state) {
			idle_loop_count = 0;
			float temp_data = temp_sensor.read_data();
			if (temp_data > - 50.0) temp = temp_data;
			als_sensor.read_data(als_data);
			new_samples_read = true;
			display_brightness.adjust_brightness(als_data[0]);
			lcd.cls();
			lcd.locate(0,0);
			lcd.printf("T %.2f,x %+4.2f,y %+4.2f,z %+4.2f",temp, x, y, z);
		} else {
			idle_loop_count += 1;
		}
		wait_ms(200);

#if MAIN_LOG_ENABLE
#if not MAIN_VERBOSE_LOG
		static unint loop_count = 0;
		if (loop_count >= 8) {
			loop_count = 0;
#endif
			pc.printf("Accel x:  %+4.3f g, y:  %+4.3f g, z:  %+4.3f g\r\n", x, y, z);
			if (new_samples_read) {
				pc.printf("Temp = %.3f deg C, ",temp);
				pc.printf("ALS ch0: %u ch1: %u", als_data[0], als_data[1]);
				pc.printf(", Display brightness %.3f\r\n", display_brightness.get_brightness());
			}
			pc.printf("accel int_cnt = %u, button int_cnt = %u, sleep_cnt = %lu\r\n", accel_int_cnt, but_int_cnt, sleep_cnt);
			pc.printf("last_sleep_duration = %d ms, total_sleep_time = %lu ms, system_up_time = %d ms\r\n", last_sleep_duration, total_sleep_time, global_tm.read_ms());
			if (device_face_down) {
				pc.printf("Device facing down\r\n");
			} else if (device_was_facing_down) {
				pc.printf("Device was facing down\r\n");
			}
#if not MAIN_VERBOSE_LOG
		} else {
			loop_count += 1;
		}
#endif
#endif

	}
}


