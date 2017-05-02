#pragma once
#include "augs/misc/machine_entropy.h"

extern bool TWEAKER_CONTROL_ENABLED;

extern float TWEAK0;
extern float TWEAK1;
extern float TWEAK2;
extern float TWEAK3;

extern int CURRENT_TWEAK;

void control_tweaker(augs::machine_entropy::local_type& in);
std::wstring write_tweaker_report();