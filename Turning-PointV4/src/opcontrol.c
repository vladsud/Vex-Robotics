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
/* BUTTON MAPPING
lift is left side triggers up and down
Spinner is right trigger down
Shooter is right side button right
Intake is right side button left
*/
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
	motorSet(leftDrivePortY, -speed);
	motorSet(leftDrivePort2, -speed);
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
	return joystickGetDigital(1, 8, JOY_UP);
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
	bool intake = false;

	bool oldState = false;

	while (true) {
		//Drive
		forward = GetForwardAxis();
		turn = GetTurnAxis();

		SetLeftDrive(forward + turn);
		SetRightDrive(forward - turn);


		//Lift
		liftUp = GetLiftUp();
		liftDown = GetLiftDown();

		if (liftUp){
			SetLiftMotor(-liftMotorSpeed);
		} else if (liftDown){
			SetLiftMotor(liftMotorSpeed/2);
		} else {
			SetLiftMotor(0);
		}

		//spinner
		spinner = GetSpinner();
			if(spinner){
				SetSpinnerMotor(spinnerMotorSpeed);
			//2948.4 is 180 degrees
		}else {
			SetSpinnerMotor(0);
		}

		auto newState = GetIntake();
		//intake
		if (oldState == false && newState == true){
			intake = !intake;
			if (intake)
				SetIntakeMotor(intakeMotorSpeed);
			else
				SetIntakeMotor(0);

		}
		oldState = newState;

		//shooter
		shooter = GetShooter();

		if(shooter){
			SetShooterMotor(shooterMotorSpeed);
		} else {
			SetShooterMotor(0);
		}
		delay(10);
	}
}



//Created by Jason Zhang, September 22, 2018
