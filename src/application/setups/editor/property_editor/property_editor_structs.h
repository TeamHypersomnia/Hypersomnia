#pragma once
#include <optional>

#include "3rdparty/imgui/imgui.h"
#include "augs/readwrite/memory_stream.h"

struct maybe_official_path;
struct property_editor_settings;

struct property_editor_state {
	std::optional<ImGuiID> last_active;
	std::string old_description = "";
};

struct property_editor_input {
	const property_editor_settings& settings;
	property_editor_state& state;
};

