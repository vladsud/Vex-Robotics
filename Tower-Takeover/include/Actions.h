#pragma once
#include "main.h"
#include "cycle.h"
#include "logger.h"

#include <math.h>
#include <cstdio>

struct Action
{
    Main &m_main = GetMain();

    // Constructor put initial actions
    Action() { m_timeStart = m_main.GetTime(); }
    unsigned int GetElapsedTime() const { return m_main.GetTime() - m_timeStart; }

    // Put when the program should stop
    virtual bool ShouldStop() = 0; //{ return true; }

    // Called after a certain amount of time
    virtual void Stop() {}
    virtual const char* Name() { return "unknown"; }
private:
    unsigned int m_timeStart;
};

struct EndOfAction : public Action
{
    bool ShouldStop() override { return false; }
};

struct WaitAction : public Action
{
    unsigned int m_wait;
    WaitAction(unsigned int wait) : m_wait(wait) {}
    bool ShouldStop() override { return GetElapsedTime() >= m_wait; }
    const char* Name() override { return "Wait"; } 
};


struct WaitTillStopsAction : public Action
{
    bool ShouldStop() override
    {
        auto left = abs(GetLeftVelocity());
        auto right = abs(GetRightVelocity());
        return left <= 5 && right <= 5;
    }
    const char* Name() override { return "WaitTillStopsAction"; } 
};
