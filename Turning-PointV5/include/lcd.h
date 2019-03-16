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
	bool m_disable = false;

  public:
	bool AtonBlueRight = false;
	bool AtonFirstPos = true;
	bool AtonClimbPlatform = true;
	bool AtonSkills = false;

	// I do not think it runs...
	LCD();
	void PrintStepInstructions();
	void SelectAction(bool rigthButton);
	void Update();
	void PrintMessage(const char *message);
};
