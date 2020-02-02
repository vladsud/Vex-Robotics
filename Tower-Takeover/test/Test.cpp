#include "test.h"
#include <cstdio>
#include <cstdarg>
#include <stdint.h>

Test* Test::m_root = nullptr;

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

unsigned int _millis()
{
    return iterations;
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
    , m_next(m_root)
    , m_name(name)
{
    m_root = this;
}

void Test::Report(const char* reason, ...)
{
    if (m_root->m_newline)
    {
        printf("\n");
        m_root->m_newline = false;
    }
    va_list args;
    va_start(args, reason);
    vprintf(reason, args);
    va_end(args);
}

void Test::Fail(const char* reason)
{
    s_Failures++;
    m_root->m_failed = true;
    Report(reason);
    throw "Error";
}

void Test::Run()
{
    if (m_root == nullptr)
    {
        printf("Nothing to run\n");
        return;
    }
    
    while (m_root != nullptr)
    {
        printf("%s...", m_root->m_name);
        try {
            m_root->m_funct();
            if (!m_root->m_failed)
                printf("Ok\n");
        } catch(...) {}
        m_root = m_root->m_next;
    }
}

int main()
{
    Test::Run();
    if (s_Failures == 0)
    {
        printf("Done\n");
        return 0;
    }
    else
    {
        printf("%d failures detected\n", s_Failures);
        return 1;
    } 
}
