#pragma once
// #include "pros/llemu.h"
#include <string>
#include "display/lvgl.h"

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

class LCD
{
public:
	bool AtonBlueRight = false;
	bool AtonFirstPos = true;
	bool AtonClimbPlatform = true;
	bool AtonSkills = false;
    bool IsTankDrive = true;

public:
	LCD();
    void PrintMessage(const char *message);
    void Update();

private:
    static lv_res_t click_action(lv_obj_t * btn);
    lv_obj_t* CreateButton(uint8_t id, const char* label, lv_obj_t* container, lv_obj_t* prevElement, bool toggled);
    void CreateControls();

private:
    char m_batteryBuffer[128];
    lv_obj_t* m_textobj = nullptr;
    lv_obj_t* m_battery = nullptr;
    unsigned int m_count = 0;

    bool lcdA;

    const Button m_buttons[4] = {
        {"Blue (right)", "Red (left)", AtonBlueRight},
        {"First", "Second", AtonFirstPos},
        {"Platform", "No Plat", AtonClimbPlatform},
        {"Tank Drive", "Arcade Drive", IsTankDrive},
    };
};