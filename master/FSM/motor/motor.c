#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include "motor.h"
#include <wiringPi.h>
#include "logging/logging.h"

/********************  Macro definitions  ********************/
#define DELAY_IN_MS		(10U)	//delay between each two consecutive motor movements
#define STEP_PIN 0 // GPIO pin 17 PIN 11
#define SLP_PIN 2 // GPIO pin 27 PIN 13
#define RST_PIN 3 // GPIO pin 22 PIN 15

/********************  Funciton definitions  ********************/
Return_t motor_init(void)
{
	wiringPiSetup();

    pinMode(STEP_PIN, OUTPUT);
    pinMode(RST_PIN, OUTPUT);
	pinMode(SLP_PIN, OUTPUT);

	/* Put the chip to sleep mode to avoid burning the motor */
	digitalWrite(SLP_PIN, LOW);
	delay(DELAY_IN_MS);
	digitalWrite(RST_PIN, LOW);
	delay(DELAY_IN_MS);
	printf("GPIO pins initialized successfully to control the motor.\n");
	return E_OK;
}

Return_t motor_move_by_angle(uint16 angle)
{
	uint64 i =0;
    
	/* Put the chip to active mode */
	digitalWrite(SLP_PIN, HIGH);
	delay(DELAY_IN_MS);
	digitalWrite(RST_PIN, HIGH);
	delay(DELAY_IN_MS);
	for (i=0; i<(uint64)(angle*SERVO_RATIO*GEAR_RATIO); i++)
	{
		digitalWrite(STEP_PIN, HIGH);
		delay(DELAY_IN_MS);
		digitalWrite(STEP_PIN, LOW);
		delay(DELAY_IN_MS);
	}
	
	printf("Motor rotation successful by angle %d\n", angle);

	/* Put the chip to sleep mode */
	digitalWrite(SLP_PIN, LOW);
	delay(DELAY_IN_MS);
	digitalWrite(RST_PIN, LOW);
	delay(DELAY_IN_MS);
	
    return E_OK;
}