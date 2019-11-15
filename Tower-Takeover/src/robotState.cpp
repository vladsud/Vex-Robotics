class StateMachine
{
    public:
        State currentState = State::Rest;
};

enum State
{
    Rest, TrayOut, ArmsUpLow, ArmsUpMid, 
};