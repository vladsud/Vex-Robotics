#pragma once

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
	bool m_RefreshOnClick = false;
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
	void PrintMessage(const char* message);
};

extern LCD g_lcd;
