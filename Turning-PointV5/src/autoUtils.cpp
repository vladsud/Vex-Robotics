#include "aton.h"
#include "actions.h"
#include "main.h"

// Not sure why this is not provided by runtime...
// This is needed for abstract virtual funcitons (which exists in Action)
extern "C" void __cxa_pure_virtual()
{
    Assert(false);
}