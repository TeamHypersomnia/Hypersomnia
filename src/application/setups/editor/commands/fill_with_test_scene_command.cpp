#include "game/cosmos/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/commands/fill_with_test_scene_command.h"

#include "test_scenes/test_scene_settings.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_player.hpp"

std::string fill_with_test_scene_command::describe() const {
	return typesafe_sprintf("Filled with %x", minimal ? "minimal test scene" : "test scene");
}

void fill_with_test_scene_command::redo(const editor_command_input in) {
	clear_undo_state();
	in.purge_selections();

	auto& work = in.folder.work;
	auto& view = in.folder.view;

	auto& mode_vars = in.folder.mode_vars;
	auto& player = in.folder.player;

	work->to_bytes(intercosm_before_fill);
	view_before_fill = augs::to_bytes(view);
	modes_before_fill = augs::to_bytes(mode_vars);
	player_before_fill = augs::to_bytes(player);

	player = {};

	test_scene_mode_vars test_vars;
	bomb_mode_vars bomb_vars;

#if IS_PRODUCTION_BUILD
	bomb_vars.warmup_secs = 45;
	bomb_vars.round_secs = 120;
	bomb_vars.freeze_secs = 15;
	bomb_vars.round_end_secs = 5;
	bomb_vars.max_rounds = 30;
#else
	bomb_vars.warmup_secs = 0;
	bomb_vars.round_secs = 200;
	bomb_vars.freeze_secs = 0;
	bomb_vars.round_end_secs = 3;
	bomb_vars.max_rounds = 30;
	bomb_vars.buy_secs_after_freeze = 60;
#endif

	const auto& settings = in.settings.test_scene;

#if BUILD_TEST_SCENES
	work->make_test_scene(in.lua, { minimal, settings.scene_tickrate }, test_vars, std::addressof(bomb_vars));
#endif
	view = {};

	mode_vars.clear();

	auto& player_id = view.ids.local_player;

	{
		const auto test_vars_id = mode_vars_id(0);
		mode_vars.get_for<test_scene_mode_vars>().try_emplace(test_vars_id, std::move(test_vars));

		if (!settings.start_bomb_mode) {
			const auto arbitrary_player_id = static_cast<mode_player_id>(0);
			player_id = arbitrary_player_id;

			player.choose_mode<test_scene_mode>(test_vars_id);
		}
	}

	{
		const auto bomb_vars_id = mode_vars_id(0);
		mode_vars.get_for<bomb_mode_vars>().try_emplace(bomb_vars_id, std::move(bomb_vars));

		if (settings.start_bomb_mode) {
			const auto arbitrary_player_id = static_cast<mode_player_id>(3);
			player_id = arbitrary_player_id;

			player.choose_mode<bomb_mode>(bomb_vars_id);
		}
	}
}

void fill_with_test_scene_command::undo(const editor_command_input in) {
	in.purge_selections();

	auto& work = in.folder.work;
	work->from_bytes(intercosm_before_fill);
	augs::from_bytes(view_before_fill, in.folder.view);
	augs::from_bytes(modes_before_fill, in.folder.mode_vars);
	augs::from_bytes(player_before_fill, in.folder.player);

	clear_undo_state();
}

void fill_with_test_scene_command::clear_undo_state() {
	intercosm_before_fill.clear();
	view_before_fill.clear();
	modes_before_fill.clear();
	player_before_fill.clear();
}
