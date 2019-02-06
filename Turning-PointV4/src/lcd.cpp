#include "lcd.h"
#include "api.h"
#include "main.h"
#include "battery.h"

LCD::LCD()
{
    // Defaults for autonomous if LCD selection is not used
    AtonBlueRight = false;
    AtonFirstPos = true;
    AtonClimbPlatform = true;

    m_buttons = 0;
    m_step = 0;
    m_RefreshOnClick = false;

    lcdInit(uart1);
    lcdClear(uart1);
    PrintStepInstructions();
    m_count = 0;
}

void LCD::PrintMessage(const char *message)
{
    m_RefreshOnClick = true;
    lcdSetBacklight(uart1, true);
    lcdSetText(uart1, 1, message);
}

void LCD::SetSkillsMode()
{
    lcdSetText(uart1, 1, "  SKILLS !!! ");
    lcdSetText(uart1, 2, "Red       Blue");
    SetSkillSelection(true);
}

void LCD::PrintStepInstructions()
{
    m_RefreshOnClick = false;
    lcdSetBacklight(uart1, true);

    Battery bat;
    float mp = bat.GetMainPower();
    float ep = bat.GetExpanderPower();
    
    if (m_step == 0)
        lcdSetText(uart1, 2, "    Continue    ");
    else if (m_step == 2)
        lcdSetText(uart1, 2, "No           Yes");
    else if (m_step != 4)
        lcdSetText(uart1, 2, "No    Back   Yes");


    switch (m_step)
    {
    case 0:
        // Print battery level
        lcdPrint(uart1, 1, "%.2f   %.2f", ep, mp);
        printf("test");
    case 1:
        // Choose side
        lcdSetText(uart1, 1, "Left     Right");
        break;
    case 2:
        // Select position
        lcdSetText(uart1, 1, "First Pos?");
        break;
    case 3:
        // Select platform
        lcdSetText(uart1, 1, "Climb platform?");
        break;
    case 4:
        // Print auton
        lcdPrint(uart1, 1, "%s, %s",
                 AtonBlueRight ? "Blue" : "Red",
                 AtonFirstPos ? "1st" : "2nd");
        lcdPrint(uart1, 2, "%-8s  Cancel",
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
        SetSkillSelection(false);
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
        m_step = 0;
        break;
    }
    m_step++;
    PrintStepInstructions();
}

void LCD::Update()
{
    m_count++;

    // Read button 
    // Returns a 3 bit integer: 100 is left, 010 is center, 001 is right
    int buttons = lcdReadButtons(uart1);

    // If the button is the same (still pressing), ignore actions
    if (m_buttons == buttons)
        return;
    m_buttons = buttons;

    ReportStatus("LCD: buttons: %d\n", buttons);

    m_count = 0;

    if (m_RefreshOnClick)
    {
        lcdSetBacklight(uart1, true);
        return;
    }

    // If nothing is clicked
    if (m_buttons == 0)
        return;

    // If not center select action based on if the button is equal to the right button
    if (!(buttons & LCD_BTN_CENTER))
    {
        ReportStatus("LCD: non-center\n");
        SelectAction(buttons & LCD_BTN_RIGHT);
        return;
    }

    // If center is pressed in step 0 then just continue to the next step
    if (m_step == 0)
    {
        m_step++;
    }

    // If presse in step 1 then select skill
    else if (m_step == 1)
    {
        ReportStatus("LCD: Skills!!!\n");
        SetSkillsMode();
    }

    // If not the last step go back
    else if (m_step != 4)
    {
        ReportStatus("LCD: Back\n");
        m_step--;
        PrintStepInstructions();
    }
}
