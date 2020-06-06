#include "main.h"
#include "intake.h"
#include "pros/motors.h"
#include "pros/misc.h"

using namespace pros;
using namespace pros::c;

extern bool joystickGetDigital(pros::controller_id_e_t id, pros::controller_digital_e_t button);

void SetIntakeMotors(int power) {
    motor_move(intakeLeftPort, power);
    motor_move(intakeRightPort, -power);
}

Intake::Intake()
    : intakeLineTracker(intakeLineTrackerPort)
{
    motor_set_brake_mode(intakeLeftPort, E_MOTOR_BRAKE_HOLD);
    motor_set_brake_mode(intakeRightPort, E_MOTOR_BRAKE_HOLD);

    motor_set_encoder_units(intakeLeftPort, pros::E_MOTOR_ENCODER_COUNTS);
    motor_set_encoder_units(intakeRightPort, pros::E_MOTOR_ENCODER_COUNTS);

    m_mode = IntakeMode::Stop;
}

void Intake::Update()
{
}
