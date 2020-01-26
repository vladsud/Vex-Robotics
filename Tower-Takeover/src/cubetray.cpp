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
{
    //motor_set_brake_mode(cubetrayPort, E_MOTOR_BRAKE_HOLD);
    motor_tare_position(cubetrayPort);
}

void CubeTray::Update()
{
    StateMachine& sm = GetStateMachine();
    currentRotation = motor_get_position(cubetrayPort) * -1;

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
                    motor = 90;
                    // motor = pid.GetPower(currentRotation, cubeTrayOut, -17, -2500); 
                }
                
                else if (currentRotation < (cubeSlowerOut + cubeTrayOut)/2)
                {
                    // Slow
                    motor = 30;
                    // motor = pid.GetPower(currentRotation, cubeTrayOut, -22, -12000);
                }
                else if (currentRotation < cubeTrayOut)
                {
                    //motor = pid.GetPower(currentRotation, cubeTrayOut, -22, -12000);
                    motor = 20;
                }
                else
                {
                    motor = 0;
                }
                // motor = (motor + m_power * 7) / 8;
                // m_power = motor;

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
            if (GetLift().get_value() < GetLift().ArmsTrayCanMoveDown && currentRotation >= restValue)
            {
                motor = -200;
            }
            break;
        case State::ArmsUpMid:
        case State::ArmsUpLow: 
            motor = pid.GetPower(currentRotation, cubeArmsUp, -7, -4000) * 200 / 127;
            break;
        case State::InitializationState: 
            // motor = pid.GetPower(currentRotation, cubeInitialization + 5, -4, -4000);
            break;
        case State::OutABit:
            if (currentRotation < outABitValue)
                motor = 100;
            else
            {
                motor = 0;
            }
            
            //motor = pid.GetPower(currentRotation, outABitValue, -1, -4000) * 200 / 127;
            break;
    }

    m_moving = (motor != 0);

    printf("Position: %f\n", currentRotation);

    // printf("m_moving: %d     current: %d     Power: %d\n", m_moving, currentRotation, motor);

    motor_move_velocity(cubetrayPort, -motor);
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
    Do(TrayAction(state), 3500);
}


void OpenTrayOnStart()
{
    // Do(LiftAction(State::InitializationState));
    SetIntake(-127);
    DoTrayAction(State::OutABit);
    // DoTrayAction(State::ArmsUpLow);;
    Wait(1250);
    DoTrayAction(State::Rest);

    // SetIntake(70);
    // Wait(500);
    // SetIntake(0);
} 