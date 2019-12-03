#include "main.h"
#include "lift.h"
#include "pros/motors.h"
#include "StateMachine.h"
#include "cubeTray.h"
#include "Actions.h"

using namespace pros;
using namespace pros::c;

Lift::Lift()
{
    motor_set_brake_mode(liftMotorPort, E_MOTOR_BRAKE_BRAKE);
}

void Lift::SetLiftMotor(int speed)
{
    motor_move(liftMotorPort, speed);
}

float Lift::get_value()
{
    return currentPosition;
}

void Lift::Update()
{

    // Get a reference of the state machine
    StateMachine& sm = GetStateMachine();
    
    // Update current Arm and Tray value
    currentPosition = motor_get_position(liftMotorPort);

    printf("Current Arm: %1f", currentPosition);

    int motor = 0;

    if (sm.GetState() == State::ArmsUpMid)
    {
        motor = pid.GetPower(currentPosition, 1300, 3, 1000, PidPrecision::LowerOk);
    }
    else if (sm.GetState() == State::ArmsUpLow)
    {
        motor = pid.GetPower(currentPosition, 1500, 3, 1000, PidPrecision::LowerOk);
    }
    else if (sm.GetState() == State::TrayOut && GetCubeTray().IsMoving())
    {
        SetLiftMotor(-30);
        m_moving = false;
        return;
    }
    else if (sm.GetState() == State::InitializationState)
    {
        motor = pid.GetPower(currentPosition, 1200, 1, 1000, PidPrecision::LowerOk);
        // printf("Position: %d  Speed: %d\n", currentArm, motor);
    }
    else if (sm.GetState() == State::Rest) 
    {
        //count++;

        motor = pid.GetPower(currentPosition, 2500, 1, 0, PidPrecision::HigerOk);
        
        /*
        if (motor == 0 && (count % 100) < 50)
        {
            motor = -20;
        }
        */
    }

    m_moving = (motor != 0);
    SetLiftMotor(motor);
    //printf("UP: %d DOWN: %d\n", goUP, goDOWN);
}

struct LiftAction : public Action
{
    LiftAction(State action)
    {
        GetStateMachine().SetState(action);
    }
    bool ShouldStop() override
    {
        return GetElapsedTime() > 100 && !GetLift().IsMoving();
    }
};

void OpenArmsOnStart()
{
    Do(LiftAction(State::InitializationState));
    Do(LiftAction(State::Rest));
}