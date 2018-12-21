#include "cycle.h"

LCD::LCD()
{
    printf("LCD Init\n");
    m_buttons = 0;
    m_step = 0;
    m_RefreshOnClick = false;
        
    AtonBlueRight = false;
    AtonFirstPos = true;
    AtonClimbPlatform = false;

    lcdInit(uart1);
    lcdClear(uart1);
    PrintStepInstructions();
    m_count = 0;
}

void LCD::PrintMessage(const char* message)
{
    m_RefreshOnClick = true;
    lcdSetBacklight(uart1, true);
    lcdSetText(uart1, 1, message);
}

void LCD::PrintStepInstructions()
{
    m_RefreshOnClick = false;
    lcdSetBacklight(uart1, true);
    if (m_step == 0)
        lcdSetText(uart1, 2, "No           Yes");
    else if (m_step != 3)
        lcdSetText(uart1, 2, "No    Back   Yes");

    switch(m_step)
    {
        case 0:
            lcdSetText(uart1, 2, "Red       Blue");
            lcdSetText(uart1, 1, "Blue (Right)?");
            break;
        case 1:
            lcdSetText(uart1, 1, "First Pos?");
            break;
        case 2:
            lcdSetText(uart1, 1, "Climb platform?");
            break;
        case 3:
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
    printf("SelectAction: %d\n", (int)rigthButton);
    switch (m_step)
    {
        case 0:
            AtonBlueRight = rigthButton;
            break;
        case 1:
            AtonFirstPos = rigthButton;
            break;
        case 2:
            AtonClimbPlatform = rigthButton;
            break;
        case 3:
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
    // save some energy
    m_count++;
    /*
    if (m_count == 1500)
    {
        m_RefreshOnClick = true;
        lcdSetBacklight(uart1, false);
    }
    */

    int buttons = lcdReadButtons(uart1);
    if (m_buttons == buttons)
        return;
    m_buttons = buttons;

    printf("LCD: step %d,  buttons: %d, count: %d\n", m_step, buttons, m_count);

    m_count = 0;

    if (m_RefreshOnClick)
    {
        lcdSetBacklight(uart1, true);
        return;
    }

    if (m_buttons == 0)
        return;

    if (!(buttons & LCD_BTN_CENTER))
    {
        SelectAction(buttons & LCD_BTN_RIGHT);
        return;
    }
    if (m_step != 0 && m_step != 3)
    {
        m_step--;
        PrintStepInstructions();
    }

}

