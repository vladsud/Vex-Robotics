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
#include "spinner.h"
#include "drive.h"
/* BUTTON MAPPING
lift is left side triggers up and down
Spinner is right trigger down
Shooter is right side button right
Intake is right side button left
*/
//Drive Control

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
	
	bool liftUp = false;
	bool liftDown = false;
	bool shooter = false;
	bool angleUp = false;
	bool angleDown = false;
	bool intake = false;

	bool oldIntakeState = false;

	
	Drive drive;
	Spinner spinner;

	int count = 0;
	while (true) {
		count++;
		if (count % 50 == 0){
			// printf("LD: %d     LL: %d     A: %d     S:%d    RL: %d     RD: %d\n", encoderGet(leftDriveEncoder), analogRead(leftLiftPotPort), analogRead(anglePotPort), analogRead(spinnerPotPort), analogRead(rightLiftPotPort), encoderGet(rightDriveEncoder));
		}


		drive.Update();

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

		spinner.DebugRun();
		spinner.Update();

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
