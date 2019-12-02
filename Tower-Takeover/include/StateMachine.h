#pragma once

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
        bool stateChange = false;
    private:
        State currentState;

};

StateMachine& GetStateMachine();
void DoTrayAction(State state);
