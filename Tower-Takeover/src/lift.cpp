#include "main.h"
#include "lift.h"
#include "pros/motors.h"
#include "StateMachine.h"
#include "cubeTray.h"
#include "intake.h"
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

    // printf("Current Arm: %.1f\n", currentPosition);

    int motor = 0;

    switch (sm.GetState()) {
        case State::ArmsUpMid:
            motor = pid.GetPower(currentPosition, ArmsMidPos, 3, 1000, PidPrecision::HigerOk);
            break;
        case State::ArmsUpLow:
            motor = pid.GetPower(currentPosition, ArmsLowPos, 3, 1000, PidPrecision::HigerOk);
            break;
        case State::TrayOut:
            // if (GetCubeTray().IsMoving())
            break;
        case State::InitializationState:
            motor = pid.GetPower(currentPosition, ArmsInitialization, 1, 500, PidPrecision::HigerOk);
            if (motor != 0)
                break;
            sm.SetState(State::Rest);
            // fall through
        case State::Rest: 
            motor = pid.GetPower(currentPosition, RestPos, 1, 0, PidPrecision::LowerOk);        
            break;
    }

    m_moving = (motor != 0);

    // hack to keep arms low
    m_count++;
    if (sm.GetState() == State::Rest && motor == 0 && (m_count % 100) < 50)
    {
        motor = 20;
    }
    
    SetLiftMotor(-motor);
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



void TowerMid()
{
    Do(LiftAction(State::ArmsUpMid));
}