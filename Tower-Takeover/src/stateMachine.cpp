#include "StateMachine.h"
#include "main.h"
#include "intake.h"
#include "cubetray.h"
#include "pros/misc.h"

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
    if ((GetTime() % 500) == 0)
        PrintController();


    // DEBUG CODE: REMOVE!!
    // currentState = State::Debug;
    // return;


    // Calculate the current state based on the current state
    State oldState = currentState;
    SetState(calculateState(currentState));

    if (currentState != oldState)
    {
        stateChange = true;
        DebugPrint();
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

void StateMachine::SetState(State s)
{
    if (currentState != State::TrayOut &&
            s == State::TrayOut)
            //GetCubeTray().get_value() >= CubeTray::restValue - 500)
        GetIntake().m_tick = 0;
    currentState = s;
}

bool StateMachine::GetStateChange()
{
    return stateChange;
}

State StateMachine::calculateState(State state)
{
    if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
        return State::Rest;

    if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
        return State::Rest;

    if (state == State::Rest || state == State::ArmsUpMid || state == State::ArmsUpLow) {
        if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
            return State::ArmsUpMid;
        if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A))
            return State::ArmsUpLow;
    }

    if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP))
    {
        if (state == State::Rest)
            return State::InitializationState;
        else if (state == State::InitializationState)
            return State::Rest;
    }

    if (state == State::Rest)
    {
        if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
            return State::TrayOut;
    }

    return state;
}

void StateMachine::DebugPrint()
{
    if (currentState == State::Rest)
    {
        ReportStatus(Log::States, "state = Rest\n");
    }
    if (currentState == State::TrayOut)
    {
        ReportStatus(Log::States, "state = TrayOut\n");
    }
    if (currentState == State::ArmsUpMid)
    {
        ReportStatus(Log::States, "state = ArmsUpMid\n");
    }
    if (currentState == State::ArmsUpLow)
    {
        ReportStatus(Log::States, "state = ArmsUpLow\n");
    }
    if (currentState == State::InitializationState)
    {
        ReportStatus(Log::States, "state =  Initialization\n");
    }
    if (currentState == State::Debug)
    {
        ReportStatus(Log::States, "state = Debug\n");
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
    if (currentState == State::Debug)
    {
        controller_print(E_CONTROLLER_MASTER, 0, 0, "Debug\n");
    }
}