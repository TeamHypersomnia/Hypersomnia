#pragma once
#include <optional>

#include "3rdparty/imgui/imgui.h"
#include "augs/readwrite/memory_stream.h"

struct property_debugger_settings;

class property_debugger_state {
	std::optional<ImGuiID> last_tweaked;

public:
	std::string old_description = "";

	void poll_change_of_active_widget();
	bool tweaked_widget_changed();
	void reset();
};

struct property_debugger_input {
	const property_debugger_settings& settings;
	property_debugger_state& state;
};

