#include "main.h"
#include "StateMachine.h"
#include "lift.h"
#include "cubetray.h"
#include "Actions.h"
#include "intake.h"
#include "lcd.h"
#include "pros/motors.h"
#include "pros/misc.h"

using namespace pros;
using namespace pros::c;

CubeTray::CubeTray() 
    : m_anglePot(cubetrayPotPort)
{
    motor_set_brake_mode(cubetrayPort, E_MOTOR_BRAKE_HOLD);
}

int CubeTray::get_value()
{
    return m_anglePot.get_value();
}

void CubeTray::Update()
{
    StateMachine& sm = GetStateMachine();
    int currentRotation = m_anglePot.get_value();

    State desiredState = sm.GetState();
    int motor = 0;
 
    // printf("current rotation: %d\n", currentRotation);
    // bool fast = true;
    switch (desiredState)
    {
        case State::TrayOut:
            isForced = false;
            // only run if the button is held down
            if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
            {
                if (currentRotation >= restValue - 500)
                {
                    GetIntake().m_tick = 0;
                    printf("ticks: %d\n", GetIntake().m_tick);
                }
            }
            if (isAuto() || controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
            {
                // Start fast, slow down half way through
                if (currentRotation > cubeSlowerOut)
                {
                    motor = pid.GetPower(cubeTrayOut, currentRotation, -7, -3000) * 200 / 127;
                    if (GetIntake().m_tick > GetIntake().tickDown)
                    {
                        if (currentRotation > cubeSlowerOut + 300)
                        {
                            SetIntake(20);
                        }
                        else
                        {
                            SetIntake(0);
                        }
                    }
                }
                else if (currentRotation > cubeTrayOut + 250)
                {
                    motor = pid.GetPower(cubeTrayOut, currentRotation, -16, -3000) * 200 / 127;
                    if (motor < 50)
                        motor = 50;
                    SetIntake(0);
                }
                else if (currentRotation > cubeTrayOut)
                {
                    motor = 20;
                }
                else
                {
                    if (!rumbled)
                    {
                        rumbled = true;
                        controller_rumble(E_CONTROLLER_MASTER, "-");
                    }
                    motor = 0;
                    m_tick = 0;
                    SetIntake(0);
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
            rumbled = false;
            pid.Reset();
            if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2) || isForced || isAuto())
            {
                if (GetLift().get_value() < GetLift().ArmsTrayCanMoveDown)
                {
                    if (currentRotation <= restValue - 100)
                        motor = -200;
                    else if (currentRotation <= restValue - 50)
                        motor = -30;
                    else
                    {
                        motor = 0;
                    }
                }
            }
            break;
        case State::ArmsUpMid:
            motor = pid.GetPower(cubeArmsUp, currentRotation, -7, -4000) * 200 / 127;
            break;
        case State::ArmsUpLow: 
            motor = pid.GetPower(cubeArmsUp, currentRotation, -7, -4000) * 200 / 127;
            break;
        case State::InitializationState:
            if (currentRotation > outABitValue)
                motor = 100;
            else
            {
                motor = 0;
            }
            
            //motor = pid.GetPower(currentRotation, outABitValue, -1, -4000) * 200 / 127;
            break;
    }

    m_moving = (motor != 0);

    // printf("isForced: %d     current: %d     power: %d\n", isForced, currentRotation, motor);

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

void DoTrayAction(State state, int timeout)
{
    Do(TrayAction(state), timeout);
}


void OpenTrayOnStart(int time)
{
    // Do(LiftAction(State::InitializationState));
    SetIntake(-127);
    DoTrayAction(State::InitializationState, 500);

    // DoTrayAction(State::ArmsUpLow);;
    Wait(time);
    DoTrayAction(State::Rest);

    // SetIntake(70);
    Wait(300);
    // SetIntake(0);
} 