#include "main.h"
#include <cstdio>
#include "pros/llemu.h"

using namespace pros::c;

void initialize()
{
  SetupMain();
  printf("Battery %.2f \n", battery_get_capacity());
  ReportStatus("Initialized\n");
}
