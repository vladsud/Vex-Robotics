#include "StateMachine.h"
#include "cycle.h"

using namespace pros;
using namespace pros::c;

// State Initialization
StateMachine::StateMachine()
{
    // Set initial state to Rest State
    currentState = State::Rest;
    
    // Calculate Arm and Tray Value
    trayValue = GetMain().cubetray.m_anglePot.get_value();
    armValue = GetMain().lift.m_anglePot.get_value();
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
        stateChange = true;
    }
    else
    {
        stateChange = false;
    }
    
}

// Return the current state
State StateMachine::GetState()
{
    return currentState;
}

State StateMachine::calculateState(State state)
{
    if (state == State::Rest)
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
            return State::ArmsUpLow;
        else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A))
            return State::ArmsUpMid;
        else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
            return State::Rest;
        else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
            return State::TrayOut;
    }
    if (state == State::TrayOut)
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
            return State::Rest;
        else
            return State::TrayOut;
    }
    if (state == State::ArmsUpMid)
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
            return State::Rest;
        else
            return State::ArmsUpMid;
    }
    if (state == State::ArmsUpLow)
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
            return State::Rest;
        else
            return State::Rest;
    }
}