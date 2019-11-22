#pragma once
#include "main.h"

enum State
{   
    Rest, TrayOut, ArmsUpLow, ArmsUpMid, InitializationState,
};

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

        int armValue;
        int trayValue;

        bool stateChange;
    private:
        State currentState;

};
