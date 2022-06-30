#pragma once
#include "augs/misc/time_utils.h"

struct editor_command_meta {
	augs::date_time timestamp;
};

class editor_setup;

struct editor_command_input {
	editor_setup& setup;
};

