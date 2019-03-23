#include "lcd.h"
#include "main.h"
#include "battery.h"
#include "aton.h"
#include "pros/llemu.h"

using namespace pros::c;

LCD::LCD()
{
    m_buttons = 0;
    m_step = 0;

    lcd_initialize();
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
    float mp = GetMainPower();

    switch (m_step)
    {
    case 0:
        lcd_print(1, "Main Battery");

        lcd_print(2, "%.2f  Cont", mp);

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
        float mp = GetMainPower();
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
