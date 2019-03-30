#include "main.h"
#include <cstdio>
#include "pros/llemu.h"

using namespace pros::c;

void initialize()
{
  SetupMain();

  lcd_initialize();
  printf("Is the LCD initialized? %d\n", lcd_is_initialized());

  printf("Battery %.2f \n", battery_get_capacity());

  ReportStatus("Initialized\n");
}
