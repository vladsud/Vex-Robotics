#include "aton.h"

const unsigned int distanceFromWall = 380;
const unsigned int midSecondFlagDistance = 53;
const unsigned int highSecondFlagDistance = 55;
const unsigned int midThirdFlagDistance = 33;
const unsigned int highThirdFlagDistance = 70;


void PrintPosition()
{
#ifndef OFFICIAL_RUN
    // PositionInfo pos = GetTracker().LatestPosition(true /*clicks*/);
    // ReportStatus("   XY=(%d, %d), A=%d\n", pos.gyro, (int)pos.X, (int)pos.Y);
#endif
}


void ResetPostionAfterHittingWall(bool leftWall)
{
    const float robotHalfSize = 7.0;

    PositionInfo pos;
    Coordinates coord;
    PositionTracker& tracker = GetTracker();
    tracker.LatestPosition(false /*clicks*/);
    coord.X = pos.X;
    coord.Y = pos.Y;

    if (leftWall)
    {
        coord.angle = -90;
        coord.X = robotHalfSize;
    }
    else
    {
        coord.angle = 0;
        coord.Y = robotHalfSize;
    }

    tracker.SetCoordinates(coord);
}


bool HitLowFlagWithRecovery(unsigned int distanceForward, int distanceBack, int angleBack = 0, int angleForward = 0)
{
    // should have different signs - positive & negative
    Assert(distanceForward > 0);
    Assert(distanceBack < 0);

    PrintPosition();
    unsigned int distance = HitTheWall(distanceForward, angleForward);

    bool recovery = false;   
    int actualAngle = GetGyro().Get();
    if (abs(actualAngle) > 5 * GyroWrapper::Multiplier || distance + 300 <= distanceForward)
    {
        recovery = true;
        unsigned int distanceAdj = 300; // min adjustment - wheels quite often spin without much movement
        if (distance + distanceAdj < distanceForward)
            distanceAdj = distanceForward - distance;
        distanceBack += distanceAdj; // distanceBack is negative

        ReportStatus("    !!! HitLowFlagWithRecovery: Recovering: a = %d, d = %d, expected d = %d, adj back = %d\n",
            actualAngle / GyroWrapper::Multiplier, distance, distanceForward, distanceAdj);
    }
    else
    {
        // ReportStatus("    Normal hit: a = %d, d = %d, expected d = %d\n", actualAngle / GyroWrapper::Multiplier, distance, distanceForward);
        ResetPostionAfterHittingWall(false /*leftWall*/);
    }

    // not using MoveExactWithAngle() here as we should avoid turning - we may get stuck
    KeepAngle keeper(angleBack);
    MoveExact(distanceBack);
    return recovery;
}


void RunSuperSkills()
{
    auto &main = GetMain();
    main.tracker.SetCoordinates({16, 60+24, -90});

    // async actions
    SetShooterAngle(true /*high*/, g_highFlagHeight-2, false /*checkPresenceOfBall*/);

    // Pick up the first ball
    GoToCapWithBallUnderIt(distanceToCap);
    // MoveExactWithLineCorrection(-2100, 450, -90);
    MoveExactWithAngle(-2100, -90);

    // Move in front of first flags
    MoveExactWithLineCorrection(2550, 700, 0);

#if 0
    // Recalibrate angle
    HitTheWall(-(int)distanceFromWall-80, -90);
    ResetPostionAfterHittingWall(true /*leftWall*/);
    MoveExact(distanceFromWall);
#endif

    // Shooting 2 balls at first row
    TurnToAngle(angleToShootFlags+1);
    ShootTwoBalls(g_highFlagHeight-2, g_midFlagHeight-5);

    // Preset for net shooting
    main.shooter.SetDistance(highSecondFlagDistance);

    ReportStatus("\nHitting 1st low flag\n");

    // Hit low flag and come back
    bool recovery = HitLowFlagWithRecovery(3200, -2800, 3 /*angleBack*/);

    // Recalibrate, and move to shooting position for second row of flags
    if (true || recovery)
    {
        HitTheWall(-(int)distanceFromWall - 150, -90);
        ResetPostionAfterHittingWall(true /*leftWall*/);
        MoveExactWithAngle(1800, -90);
    }
    else
    {
        MoveExactWithAngle(1900 - distanceFromWall + 100, -90);
    }
    

    ReportStatus("\nShooting second pole\n");

    if (main.shooter.BallStatus() != BallPresence::NoBall)
    {
        TurnToAngle(-24);
        // Shoot middle pole
        Wait(1000);
        ShootTwoBalls(highSecondFlagDistance, midSecondFlagDistance);
    }

    // Preset for net shooting
    main.shooter.SetDistance(midThirdFlagDistance);

    // pick up ball under cap
    TurnToAngle(-90);
    GoToCapWithBallUnderIt(600);

    // Flip cap #1
    MoveExactWithAngle(-400, -90);
    WaitShooterAngleToGoUp(1000); // wait for the ball
    
    IntakeDown();
    MoveWithAngle(500, -90, 40);
    MoveExactWithLineCorrection(800, 25, -90);

    // Flip cap 2
    MoveExactWithAngle(-1850, -90);
    IntakeDown();
    MoveWithAngle(500, 0, 40);
    MoveExactWithAngle(800, 0);
    MoveWithAngle(-100, 0, 45);

    ReportStatus("\nGoing after second low flag\n");

    // Low flag 2
    PrintPosition();
    //IntakeUp();

    if (false)
    {
        MoveExactWithAngle(1200, -90);
    }
    else
    {
        MoveExactWithLineCorrection(2000, 100, -90);
        MoveExactWithAngle(-550, -90);
    }
    
    HitLowFlagWithRecovery(1800, -1600, 0 /*angleBack*/, 0 /*angleForward*/);

    // Cap 3
    IntakeDown();
    MoveWithLineCorrection(1550, 930, -90);
    MoveWithAngle(600, -90, 35); // slow down a bit
    MoveExactWithAngle(800, -90);


    ReportStatus("\nGoing after 3rd low flag\n");

    //Low flag 3, shoot
    IntakeUp();
    MoveExactWithAngle(-300, -90);
    HitLowFlagWithRecovery(1700, -2200);
    TurnToAngle(-13);
    IntakeDown();
    Wait(1000);
    ShootOneBall(true/*high*/, midThirdFlagDistance, false /*checkBallPresence*/);

    if (false)
    {
        TurnToAngle(100);
        GoToCapWithBallUnderIt(800, 100);
        MoveExactWithAngle(-800, 100);
    }

    ReportStatus("\nGoing after platform\n");

    // Climb platform
    MoveExactWithAngle(-2200, 33);
    TurnToAngle(-270);
    MoveToPlatform(true);
}
