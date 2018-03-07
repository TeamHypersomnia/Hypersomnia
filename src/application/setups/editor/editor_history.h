#pragma once
#include <vector>
#include <variant>

#include "augs/templates/history.h"

#include "application/setups/editor/commands/fill_with_test_scene_command.h"
#include "application/setups/editor/commands/delete_entities_command.h"

using editor_history = augs::history<
	delete_entities_command,
	fill_with_test_scene_command
>;

using editor_command = editor_history::command_type;