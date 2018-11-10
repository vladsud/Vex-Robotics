/** @file opcontrol.c
 * @brief File for operator control code
 *
 * This file should contain the user operatorControl() function and any
 * functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "angle.h"
#include "drive.h"
#include "intake.h"
#include "lift.h"
#include "shooter.h"
#include "spinner.h"

#include "Logger.h"
#include "main.h"
// Operator Control
void operatorControl()
{
  while (true)
  {
    PrintSensors();

    UpdateAngle();
    UpdateDrive();
    UpdateIntake();
    UpdateLift();
    UpdateShooter();
    UpdateSpinner();
    
    delay(10);
  }
}

