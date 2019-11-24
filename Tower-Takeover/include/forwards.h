#pragma once

#include "stdint.h"

namespace pros { namespace c { extern "C" {
   double battery_get_capacity(void);
	void task_delay_until(uint32_t* const prev_time, const uint32_t delta);
   void task_delay(const uint32_t milliseconds);
   uint32_t millis(void);
   void delay(const uint32_t milliseconds);
}}}
