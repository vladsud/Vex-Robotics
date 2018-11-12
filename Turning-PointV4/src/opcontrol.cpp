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
	int value = joystickGetAnalog(joystick, axis);
	if (abs(value) < 15){
		value = 0;
	}
	if (value > 0){
		return float (value * value) / -127;
	} else{
		return float (value * value) / 127;
	}
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

class Spinner
{
	bool m_oldSpinnerState = false;
	bool m_on = false;
	int m_target = 0;
	int m_lastRead = 0;
	int m_lastError = 0;
	int m_power = 0;
	int m_cyclesAfterStopped = 0;
public:

	void ReadInputs()
	{
		bool newSpinnerState = GetSpinner();
		if (newSpinnerState && !m_oldSpinnerState) {
			if (!m_on)
				Start();
			else
				Stop();
			m_oldSpinnerState = newSpinnerState;
		}
	}

	void Start()
	{
		m_on = true;
		int reading = analogRead(spinnerPotPort);
		m_target = (reading > 2000) ? 520 : 3560;
		m_lastError = m_target - reading;
		m_cyclesAfterStopped = 10000; // some big number
	}

	void Stop()
	{
		if (m_on)
		{
			m_on = false;
			SetSpinnerMotor(0);
			printf("\n");
			for (int i = 0; i< 5; i++)
			{
				delay(10);
				printf(" Count:      , Read: %d  Target: %d  (stopped)\n", analogRead(spinnerPotPort), m_target);
			}
		}
	}

	void DebugRun()
	{
		static int count = 0;
		count ++;
		if (count > 200 && !(count % 500))
			Start();
	}

	void Update()
	{
		// const int maxpower = 35;

		ReadInputs();

		if (!m_on)
			return;	

		m_cyclesAfterStopped--;
		if (m_cyclesAfterStopped == 0)
		{
			Stop();
			return;
		}

		int reading = analogRead(spinnerPotPort);
		int error =  m_target - reading;
		int distance = abs(error);
		if (distance < 50 && m_cyclesAfterStopped > 10)
			m_cyclesAfterStopped = 10;
		
		// at 500, we need to slow down a lot!
		int power = error;
		if (power > 350)
			power = 350;
		else if (power < -350)
			power = -350;
		
		int differential = error - m_lastError;
		if (differential > 60)
			differential = 60;
		else if (differential < -60)
			differential = -60;
		error = differential +  m_lastError;

		m_power = power * 0.1 + differential * 0.2;

		printf("Count: %d Read: %d  Target: %d  Power: %d (%d) LastErorr: %d  Diff: %d\n", m_cyclesAfterStopped, analogRead(spinnerPotPort), m_target, m_power, power, m_lastError, error - m_lastError);
		SetSpinnerMotor(-m_power);
		m_lastError = error; 
	}
};

//Operator Control
void operatorControl() {
	float forward = 0;
	float turn = 0;

	bool liftUp = false;
	bool liftDown = false;
	bool shooter = false;
	bool angleUp = false;
	bool angleDown = false;
	bool intake = false;

	bool oldIntakeState = false;

	float ErrorPower = 0;

	Spinner spinner;

	int count = 0;
	while (true) {
		count++;
		if (count % 50 == 0){
			// printf("LD: %d     LL: %d     A: %d     S:%d    RL: %d     RD: %d\n", encoderGet(leftDriveEncoder), analogRead(leftLiftPotPort), analogRead(anglePotPort), analogRead(spinnerPotPort), analogRead(rightLiftPotPort), encoderGet(rightDriveEncoder));
		}

		//Drive
		forward = GetForwardAxis();
		turn = GetTurnAxis();
		if (turn == 0 && forward == 0){

			SetLeftDrive(0);
			SetRightDrive(0);

		} else if (turn == 0){

			float k = 0.1;

			SetLeftDrive(forward-ErrorPower);
			SetRightDrive(forward+ErrorPower);

			int error = encoderGet(leftDriveEncoder) - encoderGet(rightDriveEncoder);

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
