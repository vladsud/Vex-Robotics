#include "main.h"
#include "StateMachine.h"
#include "lift.h"
#include "cubetray.h"
#include "Actions.h"
#include "intake.h"
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

    // bool fast = true;
    switch (desiredState)
    {
        case State::TrayOut:

            // only run if the button is held down
            if (isAuto() || controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
            {
                // Start fast, slow down half way through
                if (currentRotation < cubeSlowerOut)
                {
                    // Fast
                    motor = pid.GetPower(currentRotation, cubeTrayOut, -17, -2000); 
                }
                else
                {
                    // Slow
                    motor = pid.GetPower(currentRotation, cubeTrayOut, -21, -7000);
                }
                
                motor = (motor + m_power * 7) / 8;
                m_power = motor;

            }
            /*
            else if (currentRotation < restValue + 100) 
            {
                // if tray is barely out and no buttons are pressed, reset back to original state
                sm.SetState(State::Rest);
            }
            */
            break;
        case State::Rest:
            pid.Reset();
            if (GetLift().get_value() < GetLift().ArmsTrayCanMoveDown && currentRotation >= restValue + 15)
            {
                motor = -127;
            }
            break;
        case State::ArmsUpMid:
        case State::ArmsUpLow: 
            motor = pid.GetPower(currentRotation, cubeArmsUp, -2, -4000);
            break;
        case State::InitializationState: 
            // motor = pid.GetPower(currentRotation, cubeInitialization + 5, -4, -4000);
            break;
    }

    m_moving = (motor != 0);

    printf("m_moving: %d     current: %d     Power: %d\n", m_moving, currentRotation, motor);

    if (motor > 90)
    {
        motor = 90;
    }
    motor_move(cubetrayPort, -motor);
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
    Do(TrayAction(state));
}


void OpenTrayOnStart()
{
    // Do(LiftAction(State::InitializationState));
    SetIntake(-127);
    DoTrayAction(State::Rest);
    Wait(1500);
    SetIntake(70);
    // Wait(500);
    // SetIntake(0);
} 