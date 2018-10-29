#include "main.h"

//ANALOG SENSOR PORTS
int spinnerPotPort = 2;
//END ANALOG SENSOR PORTS

//ENCODER PORTS
int leftDriveEncoderTopPort = 3;
int leftDriveEncoderBotPort = 4;
int rightDriveEncoderTopPort = 5;
int rightDriveEncoderBotPort = 6;
int leftLiftEncoderTopPort = 7;
int leftLiftEncoderBotPort = 8;
int rightLiftEncoderTopPort = 9;
int rightLiftEncoderBotPort = 10;
//END ENCODER PORTS

Encoder leftDriveEncoder;
Encoder rightDriveEncoder;
Encoder leftLiftEncoder;
Encoder rightLiftEncoder;
void initialize(){
  leftDriveEncoder = encoderInit(leftDriveEncoderTopPort, leftDriveEncoderBotPort, false);
  rightDriveEncoder = encoderInit(rightDriveEncoderTopPort, rightDriveEncoderBotPort, false);
  leftLiftEncoder = encoderInit(leftLiftEncoderTopPort, leftLiftEncoderBotPort, false);
  rightLiftEncoder = encoderInit(rightLiftEncoderTopPort, rightLiftEncoderBotPort, false);

  analogCalibrate(spinnerPotPort);
}
