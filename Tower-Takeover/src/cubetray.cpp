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
    const int currentRotation = get_value();

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
                }
            }
            if (isAuto() || controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
            {
                // Start fast, slow down half way through
                if (currentRotation > cubeSlowerOut)
                {
                    motor = pid.GetPower(cubeTrayOut, currentRotation, -7, -3000) * 200 / 127;
                    if (isAuto())
                        motor = pid.GetPower(cubeTrayOut, currentRotation, -5, -3000) * 200 / 127;
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
                    if (isAuto())
                        motor = pid.GetPower(cubeTrayOut, currentRotation, -8, -3000) * 200 / 127;
                    if (motor < 50)
                        motor = 50;
                    SetIntake(0);
                }
                else if (currentRotation > cubeTrayOut)
                {
                    motor = 25;
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

                /*
                if (m_moving ^ (motor != 0)) {
                    printf("%d: Tray diagnostics: %d, %d\n", millis(), cubeTrayOut, currentRotation);
                }
                if (motor != 0)
                    printf("   %d: Tray moving: %d %d\n", millis(), currentRotation, motor);
                */
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
                if (abs(motor_get_actual_velocity(cubetrayPort)) < 1 && currentRotation >= restValue - 100)
                    motor = 0;

            }
            break;
        case State::ArmsUpMid:
            motor = 200;
            if (currentRotation < cubeArmsUp)
                motor = 0;
            break;
        case State::ArmsUpLow: 
            motor = 200;
            if (currentRotation < cubeArmsUp)
                motor = 0;
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
    const State m_action;
    int m_timeOpened = -1;

    TrayAction(State action)
        : m_action(action)
    {
        GetStateMachine().SetState(action);
    }

    bool ShouldStop() override {
        if (GetElapsedTime() <= 100 || GetCubeTray().IsMoving())
        {
            m_timeOpened = -1;
            return false;
        }

        // We do see it being triggered too early, likely indicating open did not finish yet
        // allow two cycles to see if it really stopped moving
        if (m_timeOpened == -1)
            m_timeOpened = millis();
        if (millis() - m_timeOpened < 15)
            return false;

        return true;
    }

    const char* Name() override {
        return "TrayAction";
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
    DoTrayAction(State::InitializationState, time + 100);

    // DoTrayAction(State::ArmsUpLow);;
    Wait(time);
    DoTrayAction(State::Rest);

    // SetIntake(70);
    Wait(300);
    // SetIntake(0);
} 