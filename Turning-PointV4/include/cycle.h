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
	int m_buttons = 0;
	int m_step = 0;
	int m_count;
public:
	bool AtonBlueRight = true;
	bool AtonFirstPos = true;
	bool AtonClimbPlatform = false;
	bool AtonShootHighFlag = true;

	// I do not think it runs...
	LCD() { Init(); }
	void Init();
	void PrintStepInstructions();
	void SelectAction(bool rigthButton);
	void Update();
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

