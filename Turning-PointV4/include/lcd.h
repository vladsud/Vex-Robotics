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

	// I do not think it runs...
	LCD();
	void PrintStepInstructions();
	void SelectAction(bool rigthButton);
	void Update();
	void PrintMessage(const char *message);
	void SetSkillsMoe();
};
