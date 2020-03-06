#pragma once

#include "main.h"

struct Action
{
    // Constructor put initial actions
    Action() { m_timeStart = GetTime(); }
    unsigned int GetElapsedTime() const { return GetTime() - m_timeStart; }

    // Put when the program should stop
    virtual bool ShouldStop() = 0; //{ return true; }

    // Called when action is done - either completed or timed out
    virtual void Stop() {}
    virtual const char* Name() { return "unknown"; }
private:
    unsigned int m_timeStart;
};

bool Do(Action &&action, unsigned int timeout = 100000);
bool Do(Action &action, unsigned int timeout = 100000);

void Wait(unsigned int duration);
void WaitAfterMove(unsigned int timeout = 0);
void FinishTrayOut(unsigned int timeBegin);
