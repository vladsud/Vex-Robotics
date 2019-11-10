 
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
        {
            is_intake = 1;
        }
        else 
        {
            is_intake = 0;
            intakeFirstTime = true;
        }
    }

    if (!is_intake){
        if (intakeFirstTime)
        {
            motor_tare_position(intakeLeftPort);
            motor_tare_position(intakeRightPort);
            intakeFirstTime = false;
        }
        
        int currentError = (motor_get_position(intakeLeftPort) + motor_get_position(intakeRightPort))/2;
        totalError += currentError;

        int currentSpeed = currentError + totalError / kI;
;
        motor_move(intakeLeftPort, currentSpeed);
        motor_move(intakeRightPort, -currentSpeed);

        printf("current: %.2f  Error: %d  Speed: %d\n", (motor_get_position(intakeLeftPort) + motor_get_position(intakeRightPort))/2, currentError, currentSpeed);

        return;
    }
    
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1)) //fast intake
    {
        motor_move(intakeLeftPort, intake_normal_speed);
        motor_move(intakeRightPort, -intake_normal_speed);
    }
        
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2)) //slow outake 
    {
        motor_move(intakeLeftPort, -intake_slow_speed);
        motor_move(intakeRightPort, intake_slow_speed);
    }
}