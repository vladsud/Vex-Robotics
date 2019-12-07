#pragma once

/*******************************************************************************
* 
* LCD
*
*******************************************************************************/
struct Button
{
    const char* label;
    const char* label2;
    bool& value;
};

#define LVOBJ struct _lv_obj_t

class LCD
{
public:
	bool AtonBlue = true;
	bool AtonProtected = true;
	bool AtonClimbPlatform = true;
	bool AtonSkills = false;
    bool IsTankDrive = false;

public:
	LCD();
    void PrintMessage(const char *message);
    void Update();
    static void click_action(LVOBJ * btn);

private:
    LVOBJ* CreateButton(unsigned int id, const char* label, LVOBJ* container, LVOBJ* prevElement, bool toggled);
    void CreateControls();

private:
    char m_batteryBuffer[128];
    LVOBJ* m_textobj = nullptr;
    LVOBJ* m_battery = nullptr;
    unsigned int m_count = 0;

    const Button m_buttons[2] = {
        {"Blue", "Red", AtonBlue},
        {"Protected", "Unprotected", AtonProtected},
        // {"Tank Drive", "Arcade Drive", IsTankDrive},
    };
};

LCD& GetLcd();
