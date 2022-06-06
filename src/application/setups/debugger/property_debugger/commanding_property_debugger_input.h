#pragma once
#include "application/setups/debugger/property_debugger/property_debugger_structs.h"
#include "application/setups/debugger/debugger_command_input.h"

struct commanding_property_debugger_input {
	property_debugger_input prop_in;
	debugger_command_input command_in;
};

