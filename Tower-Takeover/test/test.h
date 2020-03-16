#pragma once
#include <cstdint>
#include "main.h"
#include "forwards.h"

void Fail(const char* reason);
void ResetTime();

#define AssertEqual(f1, f2) AssertSz((f1) == (f2), "%s == %s: %f != %f", #f1, #f2, (float)f1, (float)f2);
#define AssertLess(f1, f2)  AssertSz((f1)  < (f2), "%s < %s: %f >= %f", #f1, #f2, (float)f1, (float)f2);

using TestFunction = void();
struct Test
{
    Test(const char* name, TestFunction*  funct);

    static void Run();
    
    static void Fail(const char* reason);
    static void Report(const char* reason, ...);

private: 
    TestFunction* const m_funct;
    Test* m_next;
    const char* const m_name;
    bool m_failed = false;
    bool m_newline = true;

    static Test* m_head;
    static Test* m_tail;
};

struct TestDisabled
{
    TestDisabled(const char* name, TestFunction*  funct) {
        Test::Report("%s... Skipped\n", name);
    }
private: 
};
