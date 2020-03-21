#pragma once

#define LVOBJ struct _lv_obj_t

void DumpLogs();

enum class ButtonType
{
    ToggleButton,
    ActionButton,
};

struct ToggleButton
{
    ButtonType type = ButtonType::ToggleButton;
    const char* label;
    const char* label2;
    bool& value;
};

struct ActionButton
{
    ButtonType type = ButtonType::ActionButton;
    const char* label;
    void (*action)();
};

union Button
{
    ToggleButton toggle;
    ActionButton action;
};


/*******************************************************************************
* 
* LCD
*
*******************************************************************************/
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
    static void click_toggle(LVOBJ * btn);
    static void click_action(LVOBJ * btn);

private:
    LVOBJ* CreateButtonCore(unsigned int id, const char* label, LVOBJ* container, LVOBJ* prevElement);
    LVOBJ* CreateToggleButton(unsigned int id, const char* label, LVOBJ* container, LVOBJ* prevElement, bool toggled);
    LVOBJ* CreateActionButton(unsigned int id, const char* label, LVOBJ* container, LVOBJ* prevElement);
    void CreateControls();

private:
    char m_batteryBuffer[128];
    char m_textBuffer[128];
    LVOBJ* m_textobj = nullptr;
    LVOBJ* m_battery = nullptr;
    unsigned int m_count = 0;

    const Button m_buttons[3] = {
        {.toggle = {ButtonType::ToggleButton, "Red", "Blue", AtonRed}},
        {.toggle = {ButtonType::ToggleButton, "Protected", "Unprotected", AtonProtected}},
        {.action = {ButtonType::ActionButton, "Dump", DumpLogs}},
        // {ButtonType::ToggleButton, "Tank Drive", "Arcade Drive", IsTankDrive},
    };
};

LCD& GetLcd();
