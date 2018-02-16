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

void autonomous() {
  GoForward(70);
	MoGoDown();
	Delay(0.9);
	StopDrive();
	MoGoUp();
	GoBackward(127);
	Delay(1);
	TurnLeft(127);
	Delay(0.147);
	GoBackward(127);
	Delay(0.64);
	TurnLeft(127);
	Delay(0.5);
	GoForward(127);
	Delay(0.3);
	MoGoDown();
	Delay(0.4);
	StopDrive();
	GoBackward(127);
	Delay(1);
	StopDrive();
}


//Created by Jason Zhang, Feburary 9, 2018
