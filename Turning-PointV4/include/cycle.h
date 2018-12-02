#pragma once

#include "main.h"

#include "drive.h"
#include "intake.h"
#include "angle.h"

/*******************************************************************************
* 
* LCD
*
*******************************************************************************/
class LCD
{
public:
	bool AtonBlueRight = true;
	bool AtonFirstPos = true;
	bool AtonClimbPlatform = false; // used only if AtonFirstPos == false;

	void Init()
	{
		AtonBlueRight = true;
		AtonFirstPos = true;
		AtonClimbPlatform = false; // used only if AtonFirstPos == false;

		lcdInit(uart1);
		lcdClear(uart1);
		lcdSetBacklight(uart1, true);

		lcdInit(uart2);
		lcdClear(uart2);
		lcdSetBacklight(uart2, true);
	}

	void Update()
	{
		// int buttons = lcdReadButtons(uart2);
	}	
};

extern LCD g_lcd;

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

	int m_count = 0;

   void Update();
};

