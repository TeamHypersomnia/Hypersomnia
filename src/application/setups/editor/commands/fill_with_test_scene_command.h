#pragma once
#include <memory>
#include <string>
#include <cstddef>
#include "application/setups/editor/commands/editor_command_structs.h"

struct editor_command_input;

namespace augs {
	struct introspection_access;
}

struct fill_with_test_scene_command {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct fill_with_test_scene_command
	editor_command_common common;
private:
	std::vector<std::byte> intercosm_before_fill;
	std::vector<std::byte> view_before_fill;
	std::vector<std::byte> modes_before_fill;
	std::vector<std::byte> player_before_fill;
	bool minimal = false;
public:
	// END GEN INTROSPECTOR

	fill_with_test_scene_command() = default;
	fill_with_test_scene_command(const bool minimal) : minimal(minimal) {}

	std::string describe() const;

	void redo(editor_command_input);
	void undo(editor_command_input);

	void clear_undo_state();
};


