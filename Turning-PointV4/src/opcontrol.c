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
	return joystickGetDigital(1, 8, JOY_UP);
}
void SetShooterMotor(int speed){
	motorSet(shooterPort, -speed);
}

bool GetIntake(){
	return joystickGetDigital(1, 8, JOY_LEFT);
}
void SetIntakeMotor(int speed){
	motorSet(intakePort, speed);
}


//Angle
bool GetAngleUp(){
	return joystickGetDigital(1, 7, JOY_UP);
}

bool GetAngleDown(){
	return joystickGetDigital(1, 7, JOY_DOWN);
}

void SetAngleMotor(int speed){
	motorSet(anglePort, speed);
}



int AbsDifferent(int first, int second){
	return abs(first - second);
}

//Operator Control
void operatorControl() {
	int forward;
	int turn;

	bool liftUp;
	bool liftDown;
	bool spinner;
	bool shooter;
	bool angleUp;
	bool angleDown;
	bool intake = false;

	bool oldState = false;

	//int lastRightEncoder = 0;
	//int lastLeftEncoder = 0;

	//int ErrorPower = 0;

	while (true) {

		printf("%d\n", analogRead(anglePotPort));
		//Drive
		forward = GetForwardAxis();
		turn = GetTurnAxis();

		SetLeftDrive(forward + turn);
		SetRightDrive(forward - turn);

		/*
		if (turn == 0 && forward == 0){

			SetLeftDrive(0);
			SetRightDrive(0);

		} else if (turn == 0){

			int error = 0;
			int k = 5;

			SetLeftDrive(forward);
			SetRightDrive(forward+ErrorPower);

			error = AbsDifferent(encoderGet(leftDriveEncoder), lastLeftEncoder) - AbsDifferent(encoderGet(rightDriveEncoder), lastRightEncoder);

			ErrorPower = error/k;

			lastLeftEncoder = encoderGet(leftDriveEncoder);
			lastRightEncoder = encoderGet(rightDriveEncoder);

		} else {

			SetLeftDrive(forward + turn);
			SetRightDrive(forward - turn);

		}
		*/

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

		//angle
		angleUp = GetAngleUp();
		angleDown = GetAngleDown();

		if (angleUp){
			SetAngleMotor(angleMotorSpeed);
		} else if (angleDown){
			SetAngleMotor(angleMotorSpeed);
		} else {
			SetAngleMotor(0);
		}

		delay(10);
	}
}



//Created by Jason Zhang, September 22, 2018
