#pragma once
#include <memory>
#include <string>
#include <cstddef>
#include "application/setups/debugger/commands/debugger_command_structs.h"

struct debugger_command_input;

namespace augs {
	struct introspection_access;
}

struct fill_with_test_scene_command {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct fill_with_test_scene_command
	debugger_command_common common;
private:
	std::vector<std::byte> before_fill;
	bool minimal = false;
public:
	// END GEN INTROSPECTOR

	fill_with_test_scene_command() = default;
	fill_with_test_scene_command(const bool minimal) : minimal(minimal) {}

	std::string describe() const;

	void redo(debugger_command_input);
	void undo(debugger_command_input);

	void clear_undo_state();
};


