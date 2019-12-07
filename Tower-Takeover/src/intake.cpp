#include "main.h"
#include "intake.h"
#include "pros/motors.h"
#include "StateMachine.h"
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

    m_mode = IntakeMode::Stop;
}

bool Intake::IsCubeIn(pros::ADIAnalogIn& sensor)
{
    unsigned int sensorValue = sensor.get_value();
    int avg = (cubeIn + cubeOut)/2;
    return sensorValue > avg ? false : true;
}

void Intake::Update()
{
    StateMachine& sm = GetStateMachine();

    if (sm.GetState() == State::TrayOut && m_mode != IntakeMode::Stop)
    {
        m_mode = IntakeMode::Stop;
        printf("Stop Intake because of Tray and Arm \n");
        SetIntakeMotors(0);
        return;
    }

    bool cubeIn = IsCubeIn(intakeLineTracker);

    // Get new controller press
    if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1))
    {
        if (m_mode == IntakeMode::Intake || m_mode == IntakeMode::IntakeTower)
        {
            m_mode = IntakeMode::Hold;
        }
        else
        {
            m_mode = IntakeMode::Intake;
        }
    }
    else if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2))
    {
        if (m_mode == IntakeMode::Outtake || m_mode == IntakeMode::IntakeTower)
        {
            m_mode = IntakeMode::Hold;
        } 
        else
        {
            m_mode = IntakeMode::Outtake;
        }
    }
    else if (m_mode == IntakeMode::IntakeTower || joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP)) //slow outake 
    {
        // Start tower stacking
        m_mode = IntakeMode::IntakeTower;
        // If cube is not in slowing intake
        if (!cubeIn)
        {
            count = 0;
            SetIntakeMotors(intake_normal_speed);
        }
        // When in, stop intaking and cancel action
        else
        {
            if (count < 10)
            {
                SetIntakeMotors(intake_normal_speed);
                count++;
            }
            SetIntakeMotors(0);
            m_mode = IntakeMode::Hold;
        }
        return;
    }

    // If not intaking
    if (m_mode == IntakeMode::Hold) {
        //printf("Left: %d Right: %d LeftBool: %d RightBool %d \n", leftIntakeLineTracker.get_value(), rightIntakeLineTracker.get_value(), IsCubeIn(leftIntakeLineTracker), IsCubeIn(rightIntakeLineTracker));
        if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_LEFT))
        {
            if (!cubeIn && sm.GetState() == State::Rest)
            {
                SetIntakeMotors(-40);
            }
            else
            {
                SetIntakeMotors(0);
            }
        }
        else
        {
            SetIntakeMotors(0);
        }
        
    } 
    else if (m_mode == IntakeMode::Stop)
    {
        SetIntakeMotors(0);
    }
    else {
        // Intaking up
        if (m_mode == IntakeMode::Intake) //fast intake
        {
            SetIntakeMotors(intake_normal_speed);
        }

        //Intaking down    
        if (m_mode == IntakeMode::Outtake) //slow outake 
        {
            SetIntakeMotors(-intake_slow_speed);
        }
    }

    // printf("Left: %.1f   Right: %.1f \n", motor_get_actual_velocity(intakeLeftPort), motor_get_actual_velocity(intakeRightPort));
}

void SetIntake(int speed)
{
    if (speed > 0)
    {
        GetIntake().m_mode = IntakeMode::Intake;
    }
    else if (speed < 0)
    {
        GetIntake().m_mode = IntakeMode::Outtake;
    }
    else
    {
        GetIntake().m_mode = IntakeMode::Hold;
    }
    SetIntakeMotors(speed);
}
