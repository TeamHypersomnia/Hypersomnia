#include "game/cosmos/entity_handle.h"

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

	auto& modes = in.folder.mode_vars;
	auto& player = in.folder.player;

	work->to_bytes(intercosm_before_fill);
	view_before_fill = augs::to_bytes(view);
	modes_before_fill = augs::to_bytes(modes);
	player_before_fill = augs::to_bytes(player);

	test_scene_mode_vars test_vars;

#if BUILD_TEST_SCENES
	work->make_test_scene(in.lua, { minimal, 144 }, test_vars);
#endif
	view = {};

	auto& cosm = work->world;
	test_scene_mode test_mode;
	view.controlled_character_id = cosm[test_mode.add_player({ test_vars, cosm }, faction_type::RESISTANCE)].get_id();

	const auto test_vars_id = mode_vars_id(0);
	modes.clear();
	modes.get_for<test_scene_mode_vars>().try_emplace(test_vars_id, std::move(test_vars));

	player.current_mode_vars_id = test_vars_id;
	player.current_mode.emplace<test_scene_mode>(std::move(test_mode));
}

void fill_with_test_scene_command::undo(const editor_command_input in) {
	in.purge_selections();

	auto& work = in.folder.work;
	work->from_bytes(intercosm_before_fill);
	augs::from_bytes(view_before_fill, in.folder.view);
	augs::from_bytes(modes_before_fill, in.folder.mode_vars);
	augs::from_bytes(player_before_fill, in.folder.player);

	intercosm_before_fill.clear();
	view_before_fill.clear();
}
