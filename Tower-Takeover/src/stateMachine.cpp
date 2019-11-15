#include "StateMachine"

StateMachine::StateMachine()
{
    currentState = State::Rest;
    
    trayValue = GetMain().cubetray.m_anglePot.get_value();
    armValue = GetMain().lift.m_anglePot.get_value();
}

void StateMachine::UpdateState(State state)
{
    currentState = state;
}

void StateMachine::Update()
{
    trayValue = GetMain().cubetray.m_anglePot.get_value();
    armValue = GetMain().lift.m_anglePot.get_value();
}

State calculateState(State state)
{
    if (state == State::Rest)
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
            return State::ArmsUp;
        else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A))
            return State::ArmsMid;
        else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
            return;
        else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
            return State::TrayOut;
    }
    if (state == State::TrayOut)
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
            return State::Rest;
        else
            return;
    }
    if (state == State::ArmsMid)
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
            return State::Rest;
        else
            return;
    }
    if (state == State::ArmsLow)
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
            return State::Rest;
        else
            return;
    }
}