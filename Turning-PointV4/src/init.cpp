#include "main.h"
#include "cycle.h"

Encoder g_leftDriveEncoder = nullptr;
Encoder g_rightDriveEncoder = nullptr;
Encoder g_sideEncoder = nullptr;

Gyro g_gyro = nullptr;

Main* g_main = nullptr;

void initialize()
{
  g_leftDriveEncoder = encoderInit(leftDriveEncoderTopPort, leftDriveEncoderBotPort, true);
  g_rightDriveEncoder = encoderInit(rightDriveEncoderTopPort, rightDriveEncoderBotPort, true);
  g_sideEncoder = encoderInit(sideEncoderTopPort, sideEncoderBotPort, true);
  encoderReset(g_leftDriveEncoder);
  encoderReset(g_rightDriveEncoder);
  encoderReset(g_sideEncoder);

  // This takes .5 seconds to calibrate.
  g_gyro = gyroInit(gyroPort, 0/*multiplier, 0-default*/);
  // for some reason we get readnig of 50 after init, so worth resetting it.
  gyroReset(g_gyro);

  g_main = new Main();

  setTeamName("Fluxion");

  ReportStatus("Init\n");
}

Main& GetMain()
{
  if (g_main == nullptr)
    g_main = new Main();
  return *g_main;
}
