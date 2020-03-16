#include "test.h"
#include <cstdio>
#include <cstdarg>
#include <stdint.h>

Test* Test::m_head = nullptr;
Test* Test::m_tail = nullptr;

unsigned int s_Failures = 0;

void AssertCore(bool condition, const char *message, const char *file, int line)
{
    if (!condition)
    {
        char buffer[1024];
        snprintf(RgC(buffer), "Failed: (%s:%d) %s", file, line, message);
        Test::Fail(buffer);
    }
}

unsigned int iterations = 1;

namespace pros { namespace c {
unsigned int millis() { return iterations; }
}}

const char* LogCategoryName(Log logCategory) {
    return "";
}

void ReportStatusCore(Log logCategory, const char* format, ...)
{
    if (logCategory == Log::Warning || logCategory == Log::Error)
    {
        char buffer[1024];
        va_list args;

        va_start(args, format);
        vsnprintf(buffer, CountOf(buffer), format, args);
        va_end(args);

        Test::Fail(buffer);
    }
}


void ResetTime()
{
    iterations = 1;
}

namespace pros { namespace c {
void delay(const uint32_t milliseconds)
{
    iterations += milliseconds;
}
}}

Test::Test(const char* name, TestFunction* funct)
    : m_funct(funct)
    , m_next(nullptr)
    , m_name(name)
{
    if (m_tail)
    {
        Assert(m_tail->m_next == nullptr);
        m_tail->m_next = this;
        m_tail = this;
    } else {
        Assert(m_head == nullptr);
        m_tail = this;
        m_head = this;
    }
}

void Test::Report(const char* reason, ...)
{
    if (m_head->m_newline)
    {
        printf("\n");
        m_head->m_newline = false;
    }
    va_list args;
    va_start(args, reason);
    vprintf(reason, args);
    va_end(args);
}

void Test::Fail(const char* reason)
{
    s_Failures++;
    m_head->m_failed = true;
    Report(reason);
    
    // Uncomment if you want test to stop running after first failure
    // throw "Error";
}

void Test::Run()
{
    if (m_head == nullptr)
    {
        printf("Nothing to run\n");
        return;
    }
    
    while (m_head != nullptr)
    {
        printf("%s... ", m_head->m_name);
        try {
            ResetTime();
            m_head->m_funct();
            if (!m_head->m_failed)
                printf("Ok\n");
        } catch(...) {}
        m_head = m_head->m_next;
    }
}

int main()
{
    Test::Run();
    if (s_Failures == 0)
    {
        printf("\nAll tests passed\n");
        return 0;
    }
    else
    {
        printf("*** %d failures detected ***\n", s_Failures);
        return 1;
    } 
}
