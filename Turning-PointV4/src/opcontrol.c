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
float GetMovementJoystick(unsigned char joystick, unsigned char axis){
	float value = joystickGetAnalog(joystick, axis);
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
float GetForwardAxis(){
	return GetMovementJoystick(1, 3);
}
float GetTurnAxis(){
	return GetMovementJoystick(1, 1);
}

void SetLeftDrive(float speed){
	motorSet(leftDrivePortY, -speed);
	motorSet(leftDrivePort2, -speed);
}
void SetRightDrive(float speed){
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

void SetLiftMotor(float speed){
	motorSet(liftPort, speed);
}

//Spinner Control
bool GetSpinner(){
	return joystickGetDigital(1, 6, JOY_DOWN);
}

void SetSpinnerMotor(float speed){
	motorSet(spinnerPort, speed);
}

//Shooter Control
bool GetShooter(){
	return joystickGetDigital(1, 8, JOY_UP);
}
void SetShooterMotor(float speed){
	motorSet(shooterPort, -speed);
}

bool GetIntake(){
	return joystickGetDigital(1, 8, JOY_LEFT);
}
void SetIntakeMotor(float speed){
	motorSet(intakePort, speed);
}

//Angle
bool GetAngleUp(){
	return joystickGetDigital(1, 7, JOY_UP);
}

bool GetAngleDown(){
	return joystickGetDigital(1, 7, JOY_DOWN);
}

void SetAngleMotor(float speed){
	motorSet(anglePort, speed);
}

//Operator Control
void operatorControl() {
	float forward = 0;
	float turn = 0;

	bool liftUp = false;
	bool liftDown = false;
	bool spinner = false;
	bool shooter = false;
	bool angleUp = false;
	bool angleDown = false;
	bool intake = false;

	bool oldIntakeState = false;
	bool oldSpinnerState = false;

	float ErrorPower = 0;


	int count = 0;
	while (true) {
		count++;
		if (count % 50 == 0){
			printf("LD: %d     LL: %d     A: %d     S:%d    RL: %d     RD: %d\n", encoderGet(leftDriveEncoder), analogRead(leftLiftPotPort), analogRead(anglePotPort), analogRead(spinnerPotPort), analogRead(rightLiftPotPort), encoderGet(rightDriveEncoder));
		}

		//Drive
		forward = GetForwardAxis();
		turn = GetTurnAxis();
		if (turn == 0 && forward == 0){

			SetLeftDrive(0);
			SetRightDrive(0);

		} else if (turn == 0){

			int error = 0;
			float k = 0.1;

			SetLeftDrive(forward-ErrorPower);
			SetRightDrive(forward+ErrorPower);

			error = leftDriveEncoder - rightDriveEncoder;

			ErrorPower = error * k;

		} else {
			encoderReset(leftDriveEncoder);
			encoderReset(rightDriveEncoder);

			SetLeftDrive(forward + turn);
			SetRightDrive(forward - turn);

		}

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
		bool newSpinnerState = GetSpinner();
		if (oldSpinnerState == false && newSpinnerState == true){
			spinner = !spinner;
		}
		if(spinner)
		{
			if (analogRead(spinnerPotPort) < 2000)
			{
				while (analogRead(spinnerPotPort) < 3600)
				{
					SetSpinnerMotor(-1 * abs(analogRead(spinnerPotPort) - 3600)  * spinnerMotorConstant);
				}
				SetSpinnerMotor(0);
			}
			else
			{
				while (analogRead(spinnerPotPort) > 500)
				{
					SetSpinnerMotor(abs(analogRead(spinnerPotPort) - 450) * spinnerMotorConstant);
				}
				SetSpinnerMotor(0);
			}
			spinner = !spinner;
		}

		bool newIntakeState = GetIntake();
		//intake
		if (oldIntakeState == false && newIntakeState == true){
			intake = !intake;
			if (intake)
				SetIntakeMotor(intakeMotorSpeed);
			else
				SetIntakeMotor(0);

		}
		oldIntakeState = newIntakeState;

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
			SetAngleMotor(-angleMotorSpeed/2);
		} else {
			SetAngleMotor(0);
		}

		delay(10);
	}
}



//Created by Jason Zhang, September 22, 2018
