#include "game/transcendental/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_entity_selector.h"

#include "application/setups/editor/commands/fill_with_test_scene_command.h"

#include "test_scenes/test_scene_settings.h"

std::string fill_with_test_scene_command::describe() const {
	return typesafe_sprintf("Filled with %x", minimal ? "minimal test scene" : "test scene");
}

void fill_with_test_scene_command::redo(const editor_command_input in) {
	in.purge_selections();

	auto& work = in.folder.work;
	work->to_bytes(intercosm_before_fill);
	work->make_test_scene(in.lua, { minimal, 144 } );
}

void fill_with_test_scene_command::undo(const editor_command_input in) {
	in.purge_selections();

	auto& work = in.folder.work;
	work->from_bytes(intercosm_before_fill);
	intercosm_before_fill.clear();
}
