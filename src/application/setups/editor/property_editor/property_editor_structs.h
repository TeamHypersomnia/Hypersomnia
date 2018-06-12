#pragma once
#include <optional>

#include "3rdparty/imgui/imgui.h"
#include "augs/readwrite/memory_stream.h"
#include "view/maybe_official_path_declaration.h"

struct property_editor_settings;

class property_editor_state {
	std::optional<ImGuiID> last_tweaked;

public:
	std::string old_description = "";

	void poll_change_of_active_widget();
	bool tweaked_widget_changed();
	void reset();
};

struct property_editor_input {
	const property_editor_settings& settings;
	property_editor_state& state;
};

