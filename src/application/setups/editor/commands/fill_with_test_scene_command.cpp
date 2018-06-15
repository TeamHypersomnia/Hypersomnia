#include "game/transcendental/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/commands/fill_with_test_scene_command.h"

#include "test_scenes/test_scene_settings.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"

std::string fill_with_test_scene_command::describe() const {
	return typesafe_sprintf("Filled with %x", minimal ? "minimal test scene" : "test scene");
}

void fill_with_test_scene_command::redo(const editor_command_input in) {
	in.purge_selections();

	auto& work = in.folder.work;
	auto& view = in.folder.view;

	work->to_bytes(intercosm_before_fill);
	view_before_fill = augs::to_bytes(view);

#if BUILD_TEST_SCENES
	work->make_test_scene(in.lua, { minimal, 144 } );
#endif
	view = {};
}

void fill_with_test_scene_command::undo(const editor_command_input in) {
	in.purge_selections();

	auto& work = in.folder.work;
	work->from_bytes(intercosm_before_fill);
	augs::from_bytes(view_before_fill, in.folder.view);

	intercosm_before_fill.clear();
	view_before_fill.clear();
}
