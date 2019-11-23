#include "StateMachine.h"
#include "cycle.h"

using namespace pros;
using namespace pros::c;


// State Initialization
StateMachine::StateMachine()
{
    // Set initial state to Rest State
    currentState = State::Rest;
}

// Update
void StateMachine::Update()
{
    // Get the tray and arm value
    trayValue = GetMain().cubetray.m_anglePot.get_value();
    armValue = GetMain().lift.m_anglePot.get_value();

    // Calculate the current state based on the current state
    State oldState = currentState;
    currentState = calculateState(currentState);

    if (currentState != oldState)
    {
        DebugPrint();
    }

    //printf("Arm: %d  Tray: %d \n", armValue, trayValue);
}

// Return the current state
State StateMachine::GetState()
{
    return currentState;
}

void StateMachine::SetState(State s)
{
    currentState = s;
}

State StateMachine::calculateState(State state)
{
    if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
        return State::Rest;

    if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
        return State::Rest;

    if (state == State::Rest || state == State::ArmsUpMid || state == State::ArmsUpLow || state == State::InitializationState) {
        /* This state is not implemented yet, and causes transition to other states not to work!
        if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A))
            return State::ArmsUpLow;
        */
        if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
            return State::ArmsUpMid;
        if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A))
            return State::ArmsUpLow;
        else if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_Y))
            return State::InitializationState;
    }

    if (state == State::Rest)
    {
        if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
            return State::TrayOut;
    }

    return state;
}

void StateMachine::DebugPrint()
{
    printf("New State: ");
    if (currentState == State::Rest)
    {
        printf("Rest\n");
    }
    if (currentState == State::TrayOut)
    {
        printf("TrayOut\n");
    }
    if (currentState == State::ArmsUpMid)
    {
        printf("ArmsUpMid\n");
    }
    if (currentState == State::ArmsUpLow)
    {
        printf("ArmsUpLow\n");
    }
    if (currentState == State::InitializationState)
    {
        printf("Initialization State\n");
    }
}

void StateMachine::PrintController()
{
    //controller_clear(E_CONTROLLER_MASTER);
    if (currentState == State::Rest)
    {
        controller_print(E_CONTROLLER_MASTER, 0, 0, "Rest\n");
    }
    if (currentState == State::TrayOut)
    {
        controller_print(E_CONTROLLER_MASTER, 0, 0, "TrayOut\n");
    }
    if (currentState == State::ArmsUpMid)
    {
        controller_print(E_CONTROLLER_MASTER, 0, 0, "ArmMid\n");
    }
    if (currentState == State::ArmsUpLow)
    {
        controller_print(E_CONTROLLER_MASTER, 0, 0, "ArmLow\n");
    }
    if (currentState == State::InitializationState)
    {
        controller_print(E_CONTROLLER_MASTER, 0, 0, "Init\n");
    }
}