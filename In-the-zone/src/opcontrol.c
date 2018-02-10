//Created by Jason Zhang, Feburary 9, 2018
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

	value = (value * value) / 127;

	return value;
}

bool getMoGoUp(){
	return joystickGetDigital(1, 5, JOY_UP);
}

bool getMoGoDown(){
	return joystickGetDigital(1, 5, JOY_DOWN);
}

bool getChainUp(){
	return joystickGetDigital(1, 6, JOY_UP);
}
bool getChainDown(){
	return joystickGetDigital(1, 6, JOY_DOWN);
}

bool getClawOpen(){
	return joystickGetDigital(1, 7, JOY_UP);
}
bool getClawClose(){
	return joystickGetDigital(1, 7, JOY_DOWN);
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

void setMoGoMotor(int speed){
	motorSet(6, speed);
	motorSet(7, speed);
}

void setChainMotor(int speed){
	motorSet(8, speed);
	motorSet(9, speed);
}

void setClawMotor(int speed){
	motorSet(10, speed);
}

void operatorControl() {
	int forward;
	int turn;
	bool moGoUp;
	bool moGoDown;
	bool chainUp;
	bool chainDown;
	bool clawOpen;
	bool clawClose;

	int timerLimit = 100;
	bool clawOpening = false;

	int clawTimer;

	while (1) {
		forward = getForwardAxis();
		turn = getTurnAxis();

		setLeftDrive(forward + turn);
		setRightDrive(forward - turn);

		moGoUp = getMoGoUp();
		moGoDown = getMoGoDown();

		chainUp = getChainUp();
		chainDown = getChainDown();

		clawClose = getClawClose();
		clawOpen = getClawOpen();

		if (clawOpen || clawOpening){

			clawOpening = true;

			if(clawTimer < timerLimit){
				clawTimer++;
				setClawMotor(127);
			} else {
				setClawMotor(0);
				clawOpening = false;
			}
		}

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


		if(chainUp){
			setChainMotor(127);
		}
		else if(chainDown)
		{
			setChainMotor(-127);
		}
		else
		{
			setChainMotor(0);
		}

		if(clawClose)
		{
			setClawMotor(-30);
			clawTimer = 0;
			clawOpening = false;
		}

		delay(20);

	}
}




//Created by Jason Zhang, Feburary 9, 2018
