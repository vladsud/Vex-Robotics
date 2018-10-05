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

int leftDrivePort = 6;
int rightDrivePort = 7;

int liftMotorSpeed = 100;
int liftLeft1Port = 2;
int liftLeft2Port = 3;
int liftRight1Port = 4;
int liftRight2Port = 5;

int spinnerMotorPort = 8;
int shooterMotor1Port = 9;
int shooterMotor2Port = 10;

int rollerMotorPort = 11;
int conveyorMotorPort = 12;


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
	motorSet(leftDrivePort, speed);
}
void SetRightDrive(int speed){
	motorSet(rightDrivePort, speed);
}


//Lift Control
bool GetLiftUp(){
	return joystickGetDigital(1, 5, JOY_UP);
}

bool GetLiftDown(){
	return joystickGetDigital(1, 5, JOY_DOWN);
}

void SetLiftMotor(int speed){
	//Left1
	motorSet(liftLeft1Port, speed);
	//Left2
	motorSet(liftLeft2Port, -speed);
	//Right1
	motorSet(liftRight1Port, speed);
	//Right2
	motorSet(liftRight2Port, -speed);
}

//Spinner Control
bool GetSpinner(){
	return joystickGetDigital(1, 6, JOY_DOWN);
}

void SetSpinnerMotor(int speed){
	motorSet(spinnerMotorPort, speed);
}

//Shooter Control
bool GetShooter(){
	return joystickGetDigital(1, 8, JOY_RIGHT);
}
void SetShooterMotor(int speed){
	motorSet(shooterMotor1Port, speed);
	motorSet(shooterMotor2Port, speed);
}

//Shooter Control
bool GetIntake(){
	return joystickGetDigital(1, 8, JOY_LEFT);
}
void SetIntakeMotor(int speed){
	motorSet(rollerMotorPort, speed);
	motorSet(conveyorMotorPort, speed);
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

		intake = GetIntake();
		if(intake){
			SetIntakeMotor(100);
		} else {
			SetIntakeMotor(0);
		}
	}
}




//Created by Jason Zhang, September 22, 2018
