#include "main.h"
#include <cstdarg>

extern "C" int vsnprintf(char* buffer, size_t n, const char* format, va_list list);

struct LogCategoryInfo {
  const char* prefix;
  bool enabled;
};

static LogCategoryInfo Categories[(int)Log::Max] = {
  { "Motion: ",  false},
  { "Drive: ",   false},
  { "Gyro: ",    false},
  { "States: ",  false},
  { "",          false }, // Verbose
  { "",          true }, // info
  { "WARNING: ", true },
  { "ERROR: ",   true },
};

const char* LogCategoryName(Log logCategory) {
  Assert((int)logCategory >= 0);
  Assert((int)logCategory < (int)Log::Max);
  return Categories[(int)logCategory].prefix;
}

bool IsLoggingEnabled(Log logCategory) {
  return Categories[(int)logCategory].enabled;
}

void EnableLogs(Log logCategory)
{
  Categories[(int)logCategory].enabled = true;
}

void ReportStatusCore(Log logCategory, const char* format, ...)
{
    if (!IsLoggingEnabled(logCategory))
      return;

    char buffer[1024];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, CountOf(buffer), format, args);
    va_end(args);

    printf(buffer);

    if (logCategory == Log::Error && g_main != nullptr)
		  g_main->lcd.PrintMessage(buffer);
}
