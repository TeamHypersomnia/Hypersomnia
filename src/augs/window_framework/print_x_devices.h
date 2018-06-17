#pragma once
#include <string>
#include <X11/extensions/XInput2.h>

void print_classes(std::string& total_info, Display *display, XIAnyClassInfo **classes, int num_classes);
void print_input_devices(Display* const display);

