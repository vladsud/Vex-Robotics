#include "aton.h"
#include "atonFirstPos.h"
#include "pros/motors.h"

using namespace pros;
using namespace pros::c;

extern void SetIntakeMotors(int power);

struct LiftAction : public Action
{
    LiftAction(State action)
    {
        m_main.sm.SetState(action);
    }
    bool ShouldStop() override
    {
        return GetElapsedTime() > 100 && !m_main.lift.IsMoving();
    }
};

struct TrayAction : public Action
{
    TrayAction(State action)
    {
        m_main.sm.SetState(action);
    }

    bool ShouldStop() override {
        return GetElapsedTime() > 100 && !m_main.cubetray.IsMoving();
    }

    void Stop() override {
        printf("Stop: %d\n", m_main.cubetray.IsMoving());
    }
};

void MoveStreight(int distance, int power, int angle) {
    TurnToAngleIfNeeded(angle);
    // KeepAngle angleObj(angle);
    Do(MoveAction(distance, power));
    WaitAfterMove();
}

void SetIntake()
{
    GetMain().intake.m_mode = IntakeMode::Intake;

    SetIntakeMotors(127);
}

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonFirstPos()
{
    printf("First aton\n");

    auto &main = GetMain();
    auto timeBegin = main.GetTime();
    main.tracker.SetCoordinates({16, 60+24, -90});

    Do(LiftAction(State::InitializationState));
    Do(LiftAction(State::Rest));

    SetIntake();
    MoveStreight(4000, 60, -90);

    TurnToAngle(0);
    MoveStreight(4000, 60, 0);
    //MoveExactWithAngle(4000, -90);

    Do(TrayAction(State::TrayOut));
    Do(MoveAction(-1000, 60));
    Do(TrayAction(State::Rest));
}
 