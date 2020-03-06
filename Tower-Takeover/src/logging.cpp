#include "main.h"
#include <cstdarg>

extern "C" int vsnprintf(char* buffer, size_t n, const char* format, va_list list);

struct LogCategoryInfo {
  const char* prefix;
  bool enabledConsole = false;
  bool enabledBuffered = false;
};

static LogCategoryInfo Categories[(int)Log::Max] = {
  { "Motion: ",   false},
  { "Drive: ",    false},
  { "Gyro: ",     false, true},
  { "States: ",   false, true},
  { "Automation: ", false, true},
  { "",           false },       // Verbose
  { "",           true,  true},  // info
  { "WARNING: ",  true,  true,},
  { "ERROR: ",    true,  true },
};

/*******************************************************************************
* 
* Logging accessors
*
*******************************************************************************/
LogCategoryInfo& LogCategory(Log logCategory) {
  Assert((int)logCategory >= 0);
  Assert((int)logCategory < (int)Log::Max);
  return Categories[(int)logCategory];
}

const char* LogCategoryName(Log logCategory) {
  return LogCategory(logCategory).prefix;
}

void EnableConsoleLogs(Log logCategory)
{
  LogCategory(logCategory).enabledConsole = true;
}

void EnableBufferedLogs(Log logCategory)
{
  LogCategory(logCategory).enabledBuffered = true;
}

/*******************************************************************************
* 
* BufferedLogs
*
*******************************************************************************/
class BufferedLogs {
  // Assume  100 bytes * 500 events/sec * 60 sec = 3Mb
  const static size_t capacity = 128*1024;
  char* m_buffer = nullptr;
  char* m_curr = nullptr;
  char* m_over = nullptr;
public:

  BufferedLogs()
  {
    m_buffer = new char[capacity];
    if (m_buffer == nullptr)
    {
      printf("Was not able to allocate memory!\n");
      return;
    }
    m_over = m_buffer + capacity;
    m_curr = m_buffer;
  }

  void Add(char* string)
  {

    if (!m_curr && !isAuto())
      return;

    while (m_curr < m_over)
    {
      *m_curr = *string;
      m_curr++;
      if (!*string)
       break;
      string++;
    }
  }

  void Dump() {
    if (!m_curr)
    {
      printf("Was not able to allocate memory!\n");
      return;
    }
    printf("Dump: total %ld bytes\n\n", m_curr - m_buffer);

    // Always terminate buffer not to run past it!
    m_over[-1] = 0;
    
    char* pointer = m_buffer;
    while (pointer < m_curr)
    {
      printf(pointer);
      pointer += strlen(pointer) + 1;
    }
    printf("\nDone logging\n");
  }
};

BufferedLogs s_logs;

/*******************************************************************************
* 
* DumpLogs
*
*******************************************************************************/
void DumpLogs()
{
  s_logs.Dump();
}

/*******************************************************************************
* 
* ReportStatusCore
*
*******************************************************************************/
void ReportStatusCore(Log logCategory, const char* format, ...)
{
    auto& category = LogCategory(logCategory);
    if (!category.enabledConsole && !(isAuto() && category.enabledBuffered))
      return;

    char buffer[1024];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, CountOf(buffer), format, args);
    va_end(args);

    if (category.enabledConsole)
      printf(buffer);

    s_logs.Add(buffer);

    if (logCategory == Log::Error && g_main != nullptr)
		  g_main->lcd.PrintMessage(buffer);
}
