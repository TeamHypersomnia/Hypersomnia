#pragma once
#include <memory>
#include <string>
#include <cstddef>
#include "application/setups/editor/editor_command_structs.h"

struct editor_command_input;

struct fill_with_test_scene_command {
	// GEN INTROSPECTOR struct fill_with_test_scene_command
	editor_command_common common;
	std::vector<std::byte> intercosm_before_fill;
	bool minimal = false;
	// END GEN INTROSPECTOR

	std::string describe() const;

	void redo(editor_command_input);
	void undo(editor_command_input);
};


