#pragma once
#include <cstdint>
#include "main.h"
#include "forwards.h"

void Fail(const char* reason);
void ResetTime();

using TestFunction = void();
struct Test
{
    Test(const char* name, TestFunction*  funct);

    static void Run();
    
    static void Fail(const char* reason);
    static void Report(const char* reason, ...);

private: 
    TestFunction* const m_funct;
    Test* const m_next;
    const char* const m_name;
    bool m_failed = false;
    bool m_newline = true;

    static Test* m_root;
};

struct TestDisabled
{
    TestDisabled(const char* name, TestFunction*  funct) {
        Test::Report("%s... Skipped\n", name);
    }
private: 
};
