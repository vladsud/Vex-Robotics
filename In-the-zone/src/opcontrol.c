//Created by Jason Zhang, Feburary 16, 2018
/** @file opcontrol.c
 * @brief File for operator control code
 *
 * This file should contain the user operatorControl() function and any functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "main.h"

int getJoystick(unsigned char joystick, unsigned char axis){
	int value = joystickGetAnalog(joystick, axis);
	if (abs(value) < 15){
		value = 0;
	}
	if (value > 0){
		value = (value * value) / -127;
	} else{
		value = (value * value) / 127;
	}

	return value;
}

bool getMoGoUp(){
	return joystickGetDigital(1, 5, JOY_UP);
}

bool getMoGoDown(){
	return joystickGetDigital(1, 5, JOY_DOWN);
}
bool getMoGoHoldUp(){
	return joystickGetDigital(1, 8, JOY_UP);
}

bool getMoGoHoldDown(){
	return joystickGetDigital(1, 8, JOY_DOWN);
}

int getForwardAxis(){
	return getJoystick(1, 3);
}
int getTurnAxis(){
	return getJoystick(1, 1);
}

void setLeftDrive(int speed){
	motorSet(3, speed);
	motorSet(4, speed);
	motorSet(6, speed);
	motorSet(7, speed);
}
void setRightDrive(int speed){
	motorSet(5, speed);
	motorSet(8, speed);
	motorSet(9, speed);
}

void setMoGoMotor(int speed){
	motorSet(2, speed);
}


void operatorControl() {
	int forward;
	int turn;
	bool moGoUp;
	bool moGoDown;
	bool moGoHoldUp;
	bool moGoHoldDown;

	bool isHoldUp = false;
	bool isHoldDown = false;

	while (1) {
		forward = getForwardAxis();
		turn = getTurnAxis();

		setLeftDrive(forward + turn);
		setRightDrive(forward - turn);

		moGoUp = getMoGoUp();
		moGoDown = getMoGoDown();

		moGoHoldDown = getMoGoHoldDown();
		moGoHoldUp = getMoGoHoldUp();

		if(moGoUp){
			setMoGoMotor(127);
		}
		else if(moGoDown)
		{
			setMoGoMotor(-127);
		}
		else
		{
			setMoGoMotor(0);
		}
		if(moGoHoldUp){
			if (isHoldUp == false){
				setMoGoMotor(40);
				isHoldUp = true;
				isHoldDown = false;
			}
		}
		if(moGoHoldDown){
			if (isHoldDown == false){
				setMoGoMotor(-40);
				isHoldDown = true;
				isHoldUp = false;
			}
		}
	}
}




//Created by Jason Zhang, Feburary 16, 2018
