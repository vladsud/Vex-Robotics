#include "aton.h"
#include "atonFirstPos.h"
#include "pros/motors.h"

using namespace pros;
using namespace pros::c;

struct LiftAction : public Action
{
    LiftAction(State action)
    {
        m_main.sm.SetState(action);
    }
    bool ShouldStop() override
    {
        return !m_main.lift.IsMoving();
    }
};

struct TrayAction : public Action
{
    TrayAction(State action)
    {
        m_main.sm.SetState(action);
    }

    bool ShouldStop() override {
        return !m_main.cubetray.IsMoving();
    }
};

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonFirstPos()
{
    auto &main = GetMain();
    auto timeBegin = main.GetTime();

    main.tracker.SetCoordinates({16, 60+24, -90});

    Do(LiftAction(State::InitializationState));
    Do(LiftAction(State::Rest));

    // Do(MoveAction(2000, 60));
    MoveExactWithAngle(4000, -90);


    MoveExactWithAngle(6000, 50);
    // TurnToAngle();

    Do(TrayAction(State::TrayOut));
    Do(MoveAction(-1000, 255));
    Do(TrayAction(State::Rest));
}
 