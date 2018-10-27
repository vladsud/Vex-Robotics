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

//PORTS CHANGE HERE
int leftDrivePortY = 2;
int rightDrivePortY = 3;
int leftDrivePort2 = 4;
int rightDrivePort2 = 5;

int liftPort = 6;
int intakePort = 7;
int shooterPort = 8;
int spinnerPort = 9;
int anglePort = 10;
//END PORTS



int liftMotorSpeed = 100;
//Drive Control
int GetMovementJoystick(unsigned char joystick, unsigned char axis){
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
int GetForwardAxis(){
	return GetMovementJoystick(1, 3);
}
int GetTurnAxis(){
	return GetMovementJoystick(1, 1);
}

void SetLeftDrive(int speed){
	motorSet(leftDrivePortY, speed);
	motorSet(leftDrivePort2, speed);
}
void SetRightDrive(int speed){
	motorSet(rightDrivePortY, speed);
	motorSet(rightDrivePort2, speed);
}


//Lift Control
bool GetLiftUp(){
	return joystickGetDigital(1, 5, JOY_UP);
}

bool GetLiftDown(){
	return joystickGetDigital(1, 5, JOY_DOWN);
}

void SetLiftMotor(int speed){
	motorSet(liftPort, speed);
}

//Spinner Control
bool GetSpinner(){
	return joystickGetDigital(1, 6, JOY_DOWN);
}

void SetSpinnerMotor(int speed){
	motorSet(spinnerPort, speed);
}

//Shooter Control
bool GetShooter(){
	return joystickGetDigital(1, 8, JOY_RIGHT);
}
void SetShooterMotor(int speed){
	motorSet(shooterPort, speed);
}

bool GetIntake(){
	return joystickGetDigital(1, 8, JOY_LEFT);
}
void SetIntakeMotor(int speed){
	motorSet(intakePort, speed);
}


//Operator Control
void operatorControl() {
	int forward;
	int turn;

	bool liftUp;
	bool liftDown;
	bool spinner;
	bool shooter;
	bool intake;

	while (1) {
		//Drive
		forward = GetForwardAxis();
		turn = GetTurnAxis();

		SetLeftDrive(forward + turn);
		SetRightDrive(forward - turn);


		//Lift
		liftUp = GetLiftUp();
		liftDown = GetLiftDown();

		if (liftUp){
			SetLiftMotor(liftMotorSpeed);
		} else if (liftDown){
			SetLiftMotor(-liftMotorSpeed);
		} else {
			SetLiftMotor(0);
		}

		//spinner
		spinner = GetSpinner();
		if(spinner){
			SetSpinnerMotor(80);
		} else {
			SetSpinnerMotor(0);
		}

		//shooter
		shooter = GetShooter();
		if(shooter){
			SetShooterMotor(100);
		} else {
			SetShooterMotor(0);
		}

		//intake
		intake = GetIntake();
		if(intake){
			SetIntakeMotor(100);
		} else {
			SetIntakeMotor(0);
		}

	}
}




//Created by Jason Zhang, September 22, 2018
