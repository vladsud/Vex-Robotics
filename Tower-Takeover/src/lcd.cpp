#include "lcd.h"
#include "main.h"
#include "aton.h"
#include "forwards.h"
#include "pros/motors.h"

// Try not to include "display/lvgl.h" - it's too big and slow to compile!
// Better find individual headers that are needed, like below:
// #include "display/lvgl.h"
#include "display/lv_objx/lv_label.h"
#include "display/lv_core/lv_obj.h"
#include "display/lv_objx/lv_btn.h"

using namespace pros::c;

LCD::LCD()
{
    CreateControls();
}

void LCD::click_action(lv_obj_t * btn) 
{
    uint8_t id = lv_obj_get_free_num(btn);
    auto& value = GetLcd().m_buttons[id].value;
    value = !value;

    lv_obj_t * label = lv_obj_get_child(btn, NULL);
    
    if (value)
        lv_label_set_text(label, GetLcd().m_buttons[id].label);
    else
        lv_label_set_text(label, GetLcd().m_buttons[id].label2);
}

lv_res_t click_action(lv_obj_t * btn)
{
    LCD::click_action(btn);
    return LV_RES_OK;
}


lv_obj_t* LCD::CreateButton(unsigned int id, const char* label, lv_obj_t* container, lv_obj_t* prevElement, bool toggled)
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

    lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, ::click_action); 

    /*Add text*/
    lv_obj_t * labelEl = lv_label_create(btn, NULL);
    lv_label_set_text(labelEl, label);    

    return btn;
}

void LCD::CreateControls()
{
    m_textobj = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(m_textobj, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 100, -30);
    lv_label_set_text(m_textobj, "");

    m_battery = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(m_battery, m_textobj, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

    // auto container = lv_scr_act();
    auto container  = lv_cont_create(lv_scr_act(), NULL);

    lv_obj_t* last = nullptr;

    for (int i = 0; i < CountOf(m_buttons); i++){
        last = CreateButton(
            i,
            m_buttons[i].value ? m_buttons[i].label : m_buttons[i].label2, 
            container,
            last,
            m_buttons[i].value);
    }

    lv_cont_set_fit(container, true, true);
    lv_obj_align(container, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
}

void LCD::Update()
{
    if ((m_count % 50) == 0)
    {
        snprintf(RgC(m_batteryBuffer), "Battery %.0f %%", battery_get_capacity());
        lv_label_set_text(m_battery, m_batteryBuffer);


        static lv_style_t style_screen;
        lv_style_copy(&style_screen, &lv_style_plain);

        float tempLift = motor_get_temperature(liftMotorPort);
        float tempTray = motor_get_temperature(cubetrayPort);
        float tempIntakeLeft = motor_get_temperature(intakeLeftPort);
        float tempIntakeRight = motor_get_temperature(intakeRightPort);
        if (tempLift >= 55 || tempTray >= 55 || tempIntakeLeft >= 55 || tempIntakeRight >= 55)
        {
            style_screen.body.main_color = LV_COLOR_RED;
            style_screen.body.grad_color = LV_COLOR_RED;
        }
        else
        {
            style_screen.body.main_color = LV_COLOR_BLACK;
            style_screen.body.grad_color = LV_COLOR_BLACK;
        }


        snprintf(RgC(m_batteryBuffer), "Battery %.0f %%", battery_get_capacity());
        lv_label_set_text(m_battery, m_batteryBuffer);

        snprintf(RgC(m_textBuffer), "lift: %.0f tray: %.0f left: %.0f right: %.0f", tempLift, tempTray, tempIntakeLeft, tempIntakeRight);

        PrintMessage(m_textBuffer);

        lv_obj_set_style(lv_scr_act(), &style_screen);
    }
    m_count++;
}

void LCD::PrintMessage(const char *message)
{
    lv_label_set_text(m_textobj, message);
}
