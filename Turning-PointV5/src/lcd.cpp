#include "lcd.h"
#include "main.h"
#include "aton.h"
#include "pros/llemu.h"

using namespace pros::c;

LCD::LCD()
{
    m_buttons = 0;
    m_step = 0;

    lcd_clear();
    PrintStepInstructions();
    m_count = 0;
}

void LCD::PrintMessage(const char *message)
{
    lcd_print(1, message);
}

void LCD::PrintStepInstructions()
{
    switch (m_step)
    {
    case 0:
        lcd_print(1, "Main Battery");

        lcd_print(2, "%.2f  Cont", battery_get_capacity());

        break;
    case 1:
        if (AtonSkills)
        {
            lcd_print(1, "    SKILLS!    ");
            lcd_print(2, "     Cancel     ");
        }
        else
        {
            // Choose side
            lcd_print(1, "Left     Right");
            lcd_print(2, "Red  Skill  Blue");
        }
        break;
    case 2:
        // Select position
        lcd_print(1, "First Pos?");

        lcd_print(2, "No    Back   Yes");
        break;
    case 3:
        // Select platform
        lcd_print(1, "Climb platform?");

        lcd_print(2, "No    Back   Yes");
        break;
    case 4:
        // Print auton
        lcd_print(1, "%s, %s",
                 AtonBlueRight ? "Blue" : "Red",
                 AtonFirstPos ? "1st" : "2nd");
        lcd_print(2, "%-8s  Cancel",
                 AtonClimbPlatform ? "Climb" : "No climb");
        break;
    }
}

void LCD::SelectAction(bool rigthButton)
{
    // Choose action based on if the button clicked is the right button.
    switch (m_step)
    {
    case 1:
        AtonBlueRight = rigthButton;
        break;
    case 2:
        AtonFirstPos = rigthButton;
        break;
    case 3:
        AtonClimbPlatform = rigthButton;
        break;
    case 4:
        // Cancel
        if (!rigthButton) // no-op
            return;
        m_step = -1;
        break;
    }
    m_step++;
    PrintStepInstructions();
}

void LCD::Update()
{
    m_count++;
    if (m_step == 0 && m_count % 100 == 0)
    {
        float mp = battery_get_capacity();
        //lcdSetText(1, "Main        ");
        lcd_print(2, "%.2f  Cont", mp);

        if (mp > 10.0f)
        {
            m_disable = false;
            PrintStepInstructions();
        }
        else
        {
            lcd_print(1, "Battery");
            lcd_print(2, "NOT Connected");
            m_disable = true;
        }
    }

    if (m_disable)
        return;

    // Read button
    // Returns a 3 bit integer: 100 is left, 010 is center, 001 is right
    int buttons = lcd_read_buttons();

    // If the button is the same (still pressing), ignore actions
    if (m_buttons == buttons)
        return;
    m_buttons = buttons;

    ReportStatus("LCD: buttons: %d\n", buttons);

    // If nothing is clicked
    if (m_buttons == 0)
        return;

    // If not center select action based on if the button is equal to the right button
    if (!(buttons & LCD_BTN_CENTER))
    {
        ReportStatus("LCD: non-center\n");
        if (!AtonSkills)
            SelectAction(buttons & LCD_BTN_RIGHT);
        return;
    }

    // If center is pressed in step 0 then just continue to the next step
    if (m_step == 0)
    {
        m_step++;
    }
    else if (m_step == 1)
    {
        // If press in step 1 then select skill
        AtonSkills = !AtonSkills;
        ReportStatus("LCD: Skills toggle: %d\n", (int) AtonSkills);
        if (!AtonSkills)
            m_step--;
    }
    else if (m_step != 4)
    {
        // If not the last step go back
        ReportStatus("LCD: Back\n");
        m_step--;
    }
    PrintStepInstructions();
}



struct Button
{
    uint8_t id;
    const char* label;
    bool& value; 
};


// Starting to build alternative implementation for V5 LCD
class NewLCD
{
public:
	bool AtonBlueRight = false;
	bool AtonFirstPos = true;
	bool AtonClimbPlatform = true;
	bool AtonSkills = false;

public:
    void PrintMessage(const char *message);
    void Update();

private:
    static lv_res_t press_action(lv_obj_t * btn);
    static lv_res_t click_action(lv_obj_t * btn);
    lv_obj_t* CreateButton(uint8_t id, const char* label, lv_obj_t* container, lv_obj_t* prevElement, bool toggled);
    void CreateControls();

private:
    lv_obj_t* m_textobj = nullptr;
    lv_obj_t* m_battery = nullptr;
    unsigned int m_count = 0;

    bool lcdA;
    const Button m_buttons[3] = {
        {0, "Blue (right)", AtonBlueRight},
        {1, "Climb platform", AtonClimbPlatform},
        {2, "First pos", AtonFirstPos},
    };
};


lv_res_t NewLCD::press_action(lv_obj_t * btn) 
{
    uint8_t id = lv_obj_get_free_num(btn);
    printf("Press: %d\n)", id);
    // GetMain().lcd.m_buttons[id].value = true;
    return LV_RES_OK;
}

lv_res_t NewLCD::click_action(lv_obj_t * btn) 
{
    uint8_t id = lv_obj_get_free_num(btn);
    printf("Click: %d\n)", id);
    // GetMain().lcd.m_buttons[id].value = false;
    return LV_RES_OK;
}

lv_obj_t* NewLCD::CreateButton(uint8_t id, const char* label, lv_obj_t* container, lv_obj_t* prevElement, bool toggled)
{
    lv_obj_t * btn = lv_btn_create(container, NULL);
    lv_btn_set_toggle(btn, true);
    
    if (prevElement)
        lv_obj_align(btn, prevElement, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    else
        lv_obj_set_pos(btn, 40, 40);

    lv_cont_set_fit(btn, true, true); // enable auto-resize
    // lv_obj_set_size(btn, 100, 50);
    lv_btn_set_toggle(btn, true); // it's a toggle-button
    
    if (toggled)
        lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

    lv_obj_set_free_num(btn, id);

    lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, click_action); 
    lv_btn_set_action(btn, LV_BTN_ACTION_PR, press_action); 

    /*Add text*/
    lv_obj_t * labelEl = lv_label_create(btn, NULL);
    lv_label_set_text(labelEl, label);    

    return btn;
}

void NewLCD::CreateControls()
{
    m_textobj = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(m_textobj, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 10);

    m_battery = lv_label_create(m_textobj, NULL);
    lv_obj_align(m_battery, NULL, LV_ALIGN_OUT_TOP_MID, 0, 10);

    // auto container = lv_scr_act();
    auto container  = lv_cont_create(lv_scr_act(), NULL);
    lv_cont_set_fit(container, true, true);

    lv_obj_t* last = nullptr;
    for (int i = 0; i < CountOf(m_buttons); i++)
        last = CreateButton(i, m_buttons[i].label, container, last, m_buttons[i].value);
}

void NewLCD::Update()
{
    m_count++;
    if ((m_count % 50) == 0)
    {
        char buffer[128];
        sprintf(buffer, "Battery %.2f %%", battery_get_capacity());
        lv_label_set_text(m_battery, buffer);
    }
}

void NewLCD::PrintMessage(const char *message)
{
    lv_label_set_text(m_textobj, message);
}
