#pragma once
#include "main.h"

class StateMachine
{
    enum State
    {   
        Rest, TrayOut, ArmsUpLow, ArmsUpMid, 
    };

    public:
        StateMachine();
        void UpdateState(State state);
        State GetState();
        void Update();

        int armValue;
        int trayValue;


        bool isIntake;
    private:
        State currentState;

};

