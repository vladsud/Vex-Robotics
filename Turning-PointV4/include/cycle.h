#pragma once

#include "main.h"

#include "drive.h"
#include "intake.h"
#include "angle.h"
#include "lcd.h"
#include "gyro.h"

/*******************************************************************************
* 
* MAIN CYCLE
*
*******************************************************************************/
class Main
{
public:
	Drive drive;
	Intake intake;
	Descorer descorer;
	Shooter shooter;
	GyroWrapper gyro;

	int m_count = 0;

   void Update();
};

