#include "main.h"

Encoder leftDriveEncoder;
Encoder rightDriveEncoder;

void initialize()
{

  leftDriveEncoder = encoderInit(leftDriveEncoderTopPort, leftDriveEncoderBotPort, true);
  rightDriveEncoder = encoderInit(rightDriveEncoderTopPort, rightDriveEncoderBotPort, true);
}
