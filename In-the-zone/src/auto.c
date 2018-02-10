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
void setMoGoMotor(int speed){
	motorSet(6, speed);
	motorSet(7, speed);
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
	setMoGoMotor(127);
	delay(1000);
}
void MoGoDown(){
	setMoGoMotor(-127);
	delay(1000);
}
void Delay(int time){
	delay (time * 1000);
}

void autonomous() {
  GoForward(127);
  Delay(5);
	MoGoDown();
	Delay(5);
	StopDrive();
	MoGoUp();
	GoBackward(127);
	Delay(5);
	TurnRight(127);
	Delay(1);
	GoBackward(127);
	TurnRight(127);
	Delay(0.5);
	GoForward(127);
	Delay(3);
	MoGoDown();
	GoBackward(127);
	Delay(3);
	StopDrive();

}



//Created by Jason Zhang, Feburary 9, 2018
