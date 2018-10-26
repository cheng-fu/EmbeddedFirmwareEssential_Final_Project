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

TextLCD lcd(p19, p20, p24, p23, p22, p21); // rs, e, d4-d7

#define MAIN_LOG_ENABLE true

ALS_TSL2561 als_sensor;
Display_Brightness_Controller display_brightness;
Button button;

void DisplayOff_Callback(){
	display_state = 0;
	display_brightness.turn_off_display();
}

void reschedule_DisplayOff_TO(float TO) {

  DisplayOff_to.detach();
  if(display_state == 0)
	  display_brightness.turn_on_display();
  display_state = 1;
  DisplayOff_to.attach(&DisplayOff_Callback, TO);
}

int16_t accel_data[3];
float x, y, z;

int main()
{
	unint als_data[2] = {0,0};
	pc.baud(115200);
	pc.printf("\r\n=====================\r\n");
	pc.printf("System starting...\r\n");
	global_tm.start();
	init_Temp102();
	start_Temp102();
	init_accel_Sensor();
	init_accel_shock_detect_INT();
	enable_IRQ();
	DisplayOff_to.attach(&DisplayOff_Callback, default_DisplayOff_TO);
	while(1)
	{
		wait_ms(100);
		float temp = read_Temp102();
		wait_ms(200);
		read_accel_data(accel_data);
		x = accel_data[0] * accelSensitivity;	// x-axis acceleration in G's
		y = accel_data[1] * accelSensitivity;	// y-axis acceleration in G's
		z = accel_data[2] * accelSensitivity;	// z-axis acceleration in G's
		if (z < - 0.8) {
			device_face_down = 1;
			DisplayOff_Callback();
		} else {
			device_face_down = 0;
		}

		if (display_state) {
			als_sensor.start();
			als_sensor.read_data(als_data);
			display_brightness.adjust_brightness(als_data[0]);
		} else {
			als_sensor.stop();
		}
#if MAIN_LOG_ENABLE
		pc.printf("Temp = %.3f deg C, x:  %+4.3f g, y:  %+4.3f g, z:  %+4.3f g\r\n",temp, x, y, z);
		pc.printf("accel int_cnt = %u, button int_cnt = %u,\r\n", accel_int_cnt, but_int_cnt);
		if (display_state) {
			pc.printf("ALS ch0: %u, ch1: %u", als_data[0], als_data[1]);
			pc.printf(", Current brightness %.3f\r\n", display_brightness.get_brightness());
		}
#endif
		wait_ms(200);
		lcd.cls();
		lcd.locate(0,0);
		lcd.printf("T %.2f,x %+4.2f,y %+4.2f,z %+4.2f",temp, x, y, z);
	}
}


