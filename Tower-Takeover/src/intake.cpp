 
#include "intake.h"
using namespace pros;
using namespace pros::c;
#include "pros/motors.h"
#include <cstdlib>
 
 void Intake::Update(){
    if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1) ||
            controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2) ||
            controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1) ||
            controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2)) {
        
        if (!is_intake)
            is_intake = 1;
        else
        {
            is_intake = 0;
        }
    }

    if (!is_intake){
        int value = 0;
        printf("Actual velocity Left: %lf\n", motor_get_actual_velocity(intakeLeftPort));
        if (motor_get_actual_velocity(intakeLeftPort) < 0)
            value = 40;
        motor_move(intakeLeftPort, value);
        motor_move(intakeRightPort, -value);
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