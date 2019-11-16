#pragma once
#include "main.h"

enum State
{   
    Rest, TrayOut, ArmsUpLow, ArmsUpMid, 
};

class StateMachine
{
    public:
        StateMachine();
        State GetState();
        void Update();
        State calculateState(State state);
        void DebugPrint();

        int armValue;
        int trayValue;

        bool isIntake;
        bool stateChange;
    private:
        State currentState;

};
