#include "cycle.h"

void AssertCore(bool condition, const char* message)
{
   if (!condition)
   {
      printf("\n*** ASSERT: %s ***\n\n", message);
      g_lcd.PrintMessage(message);
   }
}

void LCD::Init()
{
    printf("LCD Init\n");
    m_buttons = 0;
    m_step = 0;
    m_RefreshOnClick = false;
        
    AtonBlueRight = true;
    AtonFirstPos = true;
    AtonClimbPlatform = false;
    AtonShootHighFlag = true;

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
    else if (m_step != 4)
        lcdSetText(uart1, 2, "No    Back   Yes");

    switch(m_step)
    {
        case 0:
            lcdSetText(uart1, 1, "Blue on Right?");
            break;
        case 1:
            lcdSetText(uart1, 1, "First Pos?");
            break;
        case 2:
            lcdSetText(uart1, 1, "Shoot High flag?");
            break;
        case 3:
            lcdSetText(uart1, 1, "Climb platform?");
            break;
        case 4:
            lcdPrint(uart1, 1, "%s, %s, %s", 
                AtonBlueRight ? "Blue" : "Red",
                AtonFirstPos ? "1st" : "2nd",
                AtonShootHighFlag ? "High" : (AtonFirstPos ? "Middle" : ""));
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
            AtonShootHighFlag = rigthButton;
            break;
        case 3:
            AtonClimbPlatform = rigthButton;
            break;
        case 4:
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
    if (m_count == 1000)
    {
        m_RefreshOnClick = true;
        lcdSetBacklight(uart1, false);
        lcdClear(uart1);
    }

    int buttons = lcdReadButtons(uart1);
    if (m_buttons == buttons)
        return;
    m_buttons = buttons;

    printf("LCD: step %d,  buttons: %d, count: %d\n", m_step, buttons, m_count);

    m_count = 0;

    if (m_RefreshOnClick)
    {
        lcdSetBacklight(uart1, true);
        PrintStepInstructions();
        return;
    }

    if (m_buttons == 0)
        return;

    if (!(buttons & LCD_BTN_CENTER))
    {
        SelectAction(buttons & LCD_BTN_RIGHT);
        return;
    }
    if (m_step != 0 && m_step != 4)
    {
        m_step--;
        PrintStepInstructions();
    }

}

