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
	if (abs(value) < 5){
		value = 0;
	}
	return value;
}
int getForwardAxis(){
	return getJoystick(1, 3);
}
int getTurnAxis(){
	return getJoystick(1, 1);
}

void setLeftDrive(int speed){
	motorSet(5, speed);
	motorSet(4, speed);
}
void setRightDrive(int speed){
	motorSet(3, speed);
	motorSet(2, speed);
}

void operatorControl() {
	int forward;
	int turn;
	while (1) {
		forward = getForwardAxis();
		turn = getTurnAxis();

		setLeftDrive(forward + turn);
		setRightDrive(forward - turn);

		delay(20);

	}
}
