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

void autonomous() {
  GoForward(127);
  delay(5000);

}
