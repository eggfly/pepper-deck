#pragma once
#include <string>

void keyboard_setup();
void keyboard_loop();
void battery_setup();
void battery_loop();

namespace ruler_deck
{
    extern std::string pressed_key;
}
