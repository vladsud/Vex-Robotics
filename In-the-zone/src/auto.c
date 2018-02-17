//Created by Jason Zhang, Feburary 9, 2018
/** @file auto.c
 * @brief File for autonomous code
 *
 * This file should contain the user autonomous() function and any functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "main.h"


void SetLeftDrive(int speed){
	motorSet(5, speed);
	motorSet(4, speed);
}
void SetRightDrive(int speed){
	motorSet(3, speed);
	motorSet(2, speed);
}
void SetMoGoMotor(int speed){
	motorSet(6, speed);
}

void GoForward(int speed){
  SetLeftDrive(speed);
  SetRightDrive(speed);
}
void GoBackward(int speed){
  SetLeftDrive(-speed);
  SetRightDrive(-speed);
}
void TurnLeft(int speed){
  SetLeftDrive(-speed);
  SetRightDrive(speed);
}
void TurnRight(int speed){
	SetLeftDrive(speed);
  SetRightDrive(-speed);
}
void StopDrive(){
	SetLeftDrive(0);
  SetRightDrive(0);
}
void MoGoUp(){
	SetMoGoMotor(127);
	delay(1000);
}
void MoGoDown(){
	SetMoGoMotor(-127);
	delay(1000);
}
void Delay(float time){
	delay (time * 1000);
}
void ResetEncoders(){
	encoderReset(encoderLeft);
	encoderReset(encoderRight);
}


void autonomous() {

	int encoderLeftValue = 0;
	int encoderRightValue = 0;

	ResetEncoders();

  GoForward(70);

	MoGoDown();

	SetMoGoMotor(40);

	while((abs(encoderLeftValue) + abs(encoderRightValue))/2 < 50){
		encoderLeftValue = encoderGet(encoderLeft)/360;
	}

	StopDrive();

	Delay(0.2);

	MoGoUp();

	ResetEncoders();

	GoBackward(70);

	while((abs(encoderLeftValue) + abs(encoderRightValue))/2 < 50){
		encoderLeftValue = encoderGet(encoderLeft)/360;
	}

	StopDrive();

	ResetEncoders();

	Delay(0.2);

	TurnLeft(70);

	while((abs(encoderLeftValue) + abs(encoderRightValue))/2 < 50){
		encoderLeftValue = encoderGet(encoderLeft)/360;
	}

	ResetEncoders();

	StopDrive();

	Delay(0.2);

	GoBackward(70);

	while((abs(encoderLeftValue) + abs(encoderRightValue))/2 < 50){
		encoderLeftValue = encoderGet(encoderLeft)/360;
	}
	StopDrive();

	ResetEncoders();

	Delay(0.2);

	TurnLeft(70);

	while((abs(encoderLeftValue) + abs(encoderRightValue))/2 < 50){
		encoderLeftValue = encoderGet(encoderLeft)/360;
	}

	StopDrive();

	ResetEncoders();

	Delay(0.2);

	GoForward(70);

	while((abs(encoderLeftValue) + abs(encoderRightValue))/2 < 50){
		encoderLeftValue = encoderGet(encoderLeft)/360;
	}

	MoGoDown();

	ResetEncoders();

	GoBackward(70);

	while((abs(encoderLeftValue) + abs(encoderRightValue))/2 < 50){
		encoderLeftValue = encoderGet(encoderLeft)/360;
	}

	StopDrive();
}































//Created by Jason Zhang, Feburary 9, 2018
