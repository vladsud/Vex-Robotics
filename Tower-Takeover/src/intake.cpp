 
#include "intake.h"
using namespace pros;
using namespace pros::c;
#include "pros/motors.h"
 
 void Intake::Update(){
    printf("is_intake: %d\n", is_intake);
    if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1) ||
            controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2) ||
            controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1) ||
            controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2)) {
        
        if (!is_intake)
            is_intake = !is_intake;
        else
        {
            is_intake = 0;
        }
    }
    if (is_intake){
        motor_move(intakeLeftPort, 20);
        motor_move(intakeRightPort, -20);
        printf("stop motor\n");
        return;
    }

    
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1)) //fast intake
    {
        intake_state = "is_intake_normal";
        motor_move(intakeLeftPort, intake_normal_speed);
        motor_move(intakeRightPort, -intake_normal_speed);
    }
        
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2)) //slow outake 
    {
        intake_state = "is_intake_slow"; 
        motor_move(intakeLeftPort, -intake_slow_speed);
        motor_move(intakeRightPort, intake_slow_speed);
    }
}