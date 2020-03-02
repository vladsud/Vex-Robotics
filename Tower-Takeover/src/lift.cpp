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
    motor_set_encoder_units(liftMotorPort, pros::E_MOTOR_ENCODER_COUNTS);
    motor_set_brake_mode(liftMotorPort, E_MOTOR_BRAKE_HOLD);
}

void Lift::SetLiftMotor(int speed)
{
    motor_move(liftMotorPort, speed);
}

int Lift::get_value()
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
            motor = pid.GetPower(currentPosition, ArmsMidPos, 3, 1000);
            GetCubeTray().isForced = true;
            break;
        case State::ArmsUpLow:
            motor = pid.GetPower(currentPosition, ArmsLowPos, 3, 1000);
            GetCubeTray().isForced = true;
            break;
        case State::TrayOut:
            break;
        case State::Rest:
            motor = pid.GetPower(currentPosition, RestPos, 1, 0, PidPrecision::LowerOk);
            /*
            // We hit plates around cube tray :(
            // So move down only if one of the following is true
            // 1) arms are too high: save to move
            // 2) tray is close to rest position: we are not hitting plates
            // 3) tray is no longer moving: likely we have some malfunction, can push through?
            auto& tray = GetCubeTray();
            if (currentPosition > RestPos + 300 ||
                    tray.m_anglePot.get_value() < CubeTray::restValue + 300 ||
                    !tray.IsMoving())
                motor = pid.GetPower(currentPosition, RestPos, 5, 0);
            */
            break;
    }

    m_moving = (motor != 0);

    /*
    // hack to keep arms low
    m_count++;
    if (sm.GetState() == State::Rest && motor == 0 && (m_count % 100) < 50)
    {
        motor = 20;
    }
    */
   
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

void DoLiftAction(State state, int timeout)
{
    Do(LiftAction(state), timeout);
}
void ArmsMid()
{
    Do(LiftAction(State::ArmsUpMid));
}

void TowerRest()
{
    Do(LiftAction(State::Rest));
}

void TowerLow()
{
    Do(LiftAction(State::ArmsUpLow));
}