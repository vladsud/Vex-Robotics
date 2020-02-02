#include "main.h"
#include "actions.h"
#include "aton.h"

template <typename T>
bool DoCore(T &&action, unsigned int timeout /* = 100000 */)
{
    auto time = _millis();
    bool timedout = false;

    while (!action.ShouldStop())
    {
        timedout = action.GetElapsedTime() >= timeout;
        if (timedout)
            break;

        MainRunUpdateCycle();

        // Check if we have bailed out of autonomous mode in manual skills (some key was pressed on joystick)
        if (ShouldBailOutOfAutonomous())
        {
            ReportStatus(Log::Info, "\n!!! Switching to manual mode!\n");
            action.Stop();
            Assert(!isAuto());
            opcontrol(); // this does not return!
            ReportStatus(Log::Error, "\n!!! Error: Should never get back from opControl()!\n");
        }
    }

    action.Stop();

    if (timedout)
    {
        ReportStatus(Log::Warning, "!!! TIME-OUT: %s: %d\n", action.Name(), timeout);
        return false;
    }
    if (timeout <= 15000)
        ReportStatus(Log::Verbose, "%s took %ld ms (time-out: %d)\n", action.Name(), _millis() - time, timeout);
    else
        ReportStatus(Log::Verbose, "%s took %ld ms\n", action.Name(), _millis() - time);
    return true;
}

bool Do(Action &&action, unsigned int timeout /* = 100000 */)
{
    return DoCore(static_cast<Action &&>(action), timeout);
}

bool Do(Action &action, unsigned int timeout /* = 100000 */)
{
    return DoCore(action, timeout);
}

struct WaitAction : public Action
{
    unsigned int m_wait;
    WaitAction(unsigned int wait) : m_wait(wait) {}
    bool ShouldStop() override { return GetElapsedTime() >= m_wait; }
    const char* Name() override { return "Wait"; } 
};

void Wait(unsigned int duration)
{
    Do(WaitAction(duration));
}
