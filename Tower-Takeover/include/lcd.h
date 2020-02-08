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
	bool AtonRed = true;
	bool AtonProtected = true;
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
    char m_textBuffer[128];
    LVOBJ* m_textobj = nullptr;
    LVOBJ* m_battery = nullptr;
    unsigned int m_count = 0;

    const Button m_buttons[2] = {
        {"Red", "Blue", AtonRed},
        {"Protected", "Unprotected", AtonProtected},
        // {"Tank Drive", "Arcade Drive", IsTankDrive},
    };
};

LCD& GetLcd();
