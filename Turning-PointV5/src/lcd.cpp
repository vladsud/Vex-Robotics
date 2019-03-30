#include "lcd.h"
#include "main.h"
#include "aton.h"

using namespace pros::c;

LCD::LCD()
{
    CreateControls();
}

lv_res_t LCD::press_action(lv_obj_t * btn) 
{
    uint8_t id = lv_obj_get_free_num(btn);
    printf("Press: %d\n)", id);
    // GetMain().lcd.m_buttons[id].value = true;
    return LV_RES_OK;
}

lv_res_t LCD::click_action(lv_obj_t * btn) 
{
    uint8_t id = lv_obj_get_free_num(btn);
    auto& value = GetMain().lcd.m_buttons[id].value;
    value = !value;;
    ReportStatus("Click: %d:  %s = %d\n)", id, GetMain().lcd.m_buttons[id].label, value);
    return LV_RES_OK;
}

lv_obj_t* LCD::CreateButton(uint8_t id, const char* label, lv_obj_t* container, lv_obj_t* prevElement, bool toggled)
{
    ReportStatus("lcd: %d %d %s\n, id, toggled, label");
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
    // lv_btn_set_action(btn, LV_BTN_ACTION_PR, press_action); 

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
    for (int i = 0; i < CountOf(m_buttons); i++)
        last = CreateButton(i, m_buttons[i].label, container, last, m_buttons[i].value);

    lv_cont_set_fit(container, true, true);
    lv_obj_align(container, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
}

void LCD::Update()
{
    if ((m_count % 50) == 0)
    {
        sprintf(m_batteryBuffer, "Battery %.0f %%", battery_get_capacity());
        lv_label_set_text(m_battery, m_batteryBuffer);
    }
    m_count++;
}

void LCD::PrintMessage(const char *message)
{
    lv_label_set_text(m_textobj, message);
}
