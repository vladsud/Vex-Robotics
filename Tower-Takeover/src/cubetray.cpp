#include "main.h"
#include "StateMachine.h"
#include "lift.h"
#include "cubetray.h"
#include "Actions.h"

#include "pros/motors.h"

using namespace pros;
using namespace pros::c;

CubeTray::CubeTray() 
    : m_anglePot(cubetrayPotPort)
{
    motor_set_brake_mode(cubetrayPort, E_MOTOR_BRAKE_HOLD);
}

void CubeTray::Update()
{
    StateMachine& sm = GetStateMachine();
    int currentRotation = GetCubeTray().m_anglePot.get_value();
    State desiredState = sm.GetState();
    int motor = 0;

    if (desiredState == State::TrayOut)
    {
        // only run if the button is held down
        if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
        {
            if (currentRotation < midValue - 300)
            {
                motor = pid.GetPower(currentRotation, upValue, -16, -2000);
            }
            else
            {
                motor = pid.GetPower(currentRotation, upValue, -19, -8000);
            }
            

        }
        // printf("Moving out: %d %d\n", currentRotation, motor);
    }
    else if (desiredState == State::Rest)
    {
        if (GetLift().get_value() < GetLift().ArmsLowPos && currentRotation >= restValue + 15)
        {
            motor = -127;
        }
        //printf("Rotation: %d", currentRotation);
    }
    else if (desiredState == State::ArmsUpMid || desiredState == State::ArmsUpLow) 
    {
        motor = pid.GetPower(currentRotation, midValue, -2, -4000);
    }
    else if (desiredState == State::InitializationState) 
    {
        //printf("Move Tray Now");
        motor = pid.GetPower(currentRotation, midValue, -4, -4000);
    }
    else if (sm.GetState() == State::Debug)
    {
        motor = 0;
    }

    m_moving = (motor != 0);

    // printf("m_moving: %d     current: %d     Power: %d\n", m_moving, currentRotation, motor);
    motor_move(cubetrayPort, motor);
}

struct TrayAction : public Action
{
    TrayAction(State action)
    {
        GetStateMachine().SetState(action);
    }

    bool ShouldStop() override {
        return GetElapsedTime() > 100 && !GetCubeTray().IsMoving();
    }

    void Stop() override {
    }
};

void DoTrayAction(State state)
{
    Do(TrayAction(State::TrayOut));
}
