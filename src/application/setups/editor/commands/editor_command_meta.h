#pragma once
#include "augs/misc/date_time.h"

struct editor_command_meta {
	augs::date_time timestamp;
	bool is_child = false;
};

class editor_setup;
struct editor_settings;

struct editor_command_input {
	editor_setup& setup;
	editor_settings& settings;
	bool skip_inspector;
};

