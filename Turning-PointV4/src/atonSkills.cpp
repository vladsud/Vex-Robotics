#include "aton.h"

const unsigned int distanceFromWall = 380;


void PrintPosition()
{
#ifndef OFFICIAL_RUN
    PositionInfo pos;
    GetTracker().LatestPosition(true /*clicks*/);
    ReportStatus("   XY=(%d, %d), A=%d\n", pos.gyro, (int)pos.X, (int)pos.Y);
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


unsigned int HitTheWall(int distanceForward, int angle)
{
    Assert(distanceForward != 0);
    Drive& drive = GetMain().drive;

    TurnToAngleIfNeeded(angle);

    ReportStatus("Hitting wall: a=%d d1=%d\n", angle, distanceForward);

    int distance = distanceForward * 9 / 10 - Sign(distanceForward) * 250;
    if (distance * distanceForward <= 0)
        distance = Sign(distanceForward) * 100;

    KeepAngle keeper(angle);

    Move(distance, 85, true /*StopOnColision */);
    unsigned int distanceTravelled = drive.m_distance;

    MoveTimeBased(35 * Sign(distanceForward), 5000, true /*waitForStop*/); // attempt to fully stop, for more accurate back movement
    Wait(100);
    distanceTravelled += drive.m_distance;

    ReportStatus("   Actually travelled: %d\n", distanceTravelled);

    return distanceTravelled;
}


void HitLowFlagWithRecovery(unsigned int distanceForward, int distanceBack, int angleBack = 0, int angleForward = 0)
{
    Assert(distanceBack < 0); // should have different signs - positive & negative

    PrintPosition();
    unsigned int distance = HitTheWall(distanceForward, angleForward);

    int actualAngle = GetGyro().Get();
    if (abs(actualAngle) > 6 * GyroWrapper::Multiplier || distance + 300 <= distanceForward)
    {
        ReportStatus("    !!! HitLowFlagWithRecovery: Recovering: a = %d, d = %d, expected d = %d\n", actualAngle, distance, distanceForward);
        if (distance < distanceForward)
            distanceBack -= (distanceForward - distance);
    }
    else
    {
        ReportStatus("    Normal hit: Recovering: a = %d, d = %d, expected d = %d\n", actualAngle, distance, distanceForward);
        ResetPostionAfterHittingWall(false /*leftWall*/);
    }

    MoveExactWithAngle(distanceBack, angleBack);
}


void RunSuperSkills()
{
    auto &main = GetMain();
    main.tracker.SetCoordinates({16, 60+24, -90});

    // Pick up the first ball
    GoToCapWithBallUnderIt();
    MoveExactWithAngle(-1930, -90);

    // Move in front of first flags
    MoveExactWithAngle(2600, 0);

    // Recalibrate angle
    HitTheWall(-(int)distanceFromWall-120, -90);
    ResetPostionAfterHittingWall(true /*leftWall*/);
    MoveExact(distanceFromWall);

    // Shooting 2 balls at first row
    TurnToFlagsAndShootTwoBalls();


    ReportStatus("\nHitting 1st low flag\n");

    // Hit low flag and come back
    HitLowFlagWithRecovery(2750, -2800, 3 /*angleBack*/);

    // Recalibrate, and move to shooting position for second row of flags
    HitTheWall(-(int)distanceFromWall - 50, -90);
    ResetPostionAfterHittingWall(true /*leftWall*/);


    ReportStatus("\nShooting second pole\n");

    MoveExactWithAngle(2050, -90);
    TurnToAngle(-24);

    // Shoot middle pole
    ShootTwoBalls(63, 115);

    // pick up ball under cap
    TurnToAngle(-90);
    GoToCapWithBallUnderIt(400);

    // Flip cap #1
    MoveExactWithAngle(-350, -90);
    Wait(300);
    IntakeDown();
    MoveExactWithAngle(1300, -90);

    //Flip cap 2
    MoveExactWithAngle(-1800, -90);
    IntakeDown();
    MoveExactWithAngle(1300, 0);

    ReportStatus("\nGoing after second low flag\n");

    // Low flag 2
    PrintPosition();
    //IntakeUp();
    MoveExactWithAngle(1140, -90);
    HitLowFlagWithRecovery(1250, -1650, 0, 2);

    //Cap 3
    TurnToAngle(-90);
    IntakeDown();
    MoveWithAngle(1580, -90);
    MoveWithAngle(500, -90, 30); // slow down a bit
    MoveExactWithAngle(800, -90);

    ReportStatus("\nGoing after 3rd low flag\n");

    //Low flag 3, shoot
    IntakeUp();
    MoveExactWithAngle(-200, -90);
    HitLowFlagWithRecovery(1400, -2200);
    TurnToAngle(-13);
    IntakeDown();
    ShootTwoBalls(33, 70);

    ReportStatus("\nGoing after platform\n");

    // Climb platform
    MoveExactWithAngle(-2200, 30);
    TurnToAngle(-270);
    MoveToPlatform(true);
}
