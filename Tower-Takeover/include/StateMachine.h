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

        int armValue;
        int trayValue;


        bool isIntake;
    private:
        State currentState;

};
