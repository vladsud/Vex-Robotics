#include "main.h"

void memmove(char* dest, char* src, size_t size)
{
    while (size > 0)
    {
        *dest = *src;
        size--;
        dest++;
        src++;
    }
}

// Scans both joysticks, allowing secondary operator to help with controlling non-driving functions.
bool joystickGetDigital(unsigned char buttonGroup, unsigned char button)
{
	return ::joystickGetDigital(1, buttonGroup, button) || ::joystickGetDigital(2, buttonGroup, button);
}


