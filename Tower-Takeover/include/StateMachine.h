#pragma once
//#include "okapi/api.hpp"

enum State
{   
    Rest, TrayOut, ArmsUpLow, ArmsUpMid, InitializationState, DoubleCube, Debug
};

// extern std::shared_ptr<okapi::OdomChassisController> chassis;

class StateMachine
{
    public:
        StateMachine();
        State GetState();
        void Update();
        State calculateState(State state);
        void DebugPrint();
        void SetState(State s);
        void PrintController();

        bool GetStateChange();
        bool stateChange = false;
    private:
        State currentState;

};

StateMachine& GetStateMachine();
void DoTrayAction(State state, int timeout = 3500);
void DoLiftAction(State state, int timeout = 3500);
