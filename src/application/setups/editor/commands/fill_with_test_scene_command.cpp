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
	/* Reset to zero if it happened while playtesting */
	common.when_happened = 0;

	clear_undo_state();
	in.purge_selections();

	auto& work = in.folder.commanded->work;
	auto& view = in.folder.view;
	auto& view_ids = in.folder.commanded->view_ids;
	auto& player = in.folder.player;
	auto& mode_vars = in.folder.commanded->mode_vars;

	auto ms = augs::ref_memory_stream(before_fill);

	augs::write_bytes(ms, *in.folder.commanded);
	augs::write_bytes(ms, view);
	augs::write_bytes(ms, player);

	player = {};
	view = {};
	view_ids = {};

	test_scene_mode_vars test_vars;
	bomb_mode_vars bomb_vars;

#if IS_PRODUCTION_BUILD
	bomb_vars.warmup_secs = 45;
	bomb_vars.round_secs = 120;
	bomb_vars.freeze_secs = 15;
	bomb_vars.round_end_secs = 5;
	bomb_vars.max_rounds = 30;
#else
	bomb_vars.warmup_secs = 30;
	bomb_vars.round_secs = 200;
	bomb_vars.freeze_secs = 0;
	bomb_vars.round_end_secs = 3;
	bomb_vars.max_rounds = 4;
	bomb_vars.buy_secs_after_freeze = 60;
	bomb_vars.match_summary_seconds = 5;
#endif

	const auto& settings = in.settings.test_scene;

#if BUILD_TEST_SCENES
	work.make_test_scene(in.lua, { minimal, settings.scene_tickrate }, test_vars, std::addressof(bomb_vars));
#endif

	auto& vars = mode_vars.vars;
	vars.clear();

	auto& player_id = view.local_player;

	{
		const auto test_vars_id = raw_mode_vars_id(0);
		vars.get_for<test_scene_mode>().try_emplace(test_vars_id, std::move(test_vars));

		if (!settings.start_bomb_mode) {
			const auto arbitrary_player_id = static_cast<mode_player_id>(0);
			player_id = arbitrary_player_id;

			auto& def = mode_vars.default_mode;
			def.type_id.set<test_scene_mode>();
			def.raw = test_vars_id;
		}
	}

	{
		const auto bomb_vars_id = raw_mode_vars_id(0);
		vars.get_for<bomb_mode>().try_emplace(bomb_vars_id, std::move(bomb_vars));

		if (settings.start_bomb_mode) {
			const auto arbitrary_player_id = static_cast<mode_player_id>(3);
			player_id = arbitrary_player_id;

			auto& def = mode_vars.default_mode;
			def.type_id.set<bomb_mode>();
			def.raw = bomb_vars_id;
		}
	}
}

void fill_with_test_scene_command::undo(const editor_command_input in) {
	in.purge_selections();

	auto ms = augs::cref_memory_stream(before_fill);

	in.folder.commanded->work.clear();

	augs::read_bytes(ms, *in.folder.commanded);
	augs::read_bytes(ms, in.folder.view);
	augs::read_bytes(ms, in.folder.player);

	clear_undo_state();
}

void fill_with_test_scene_command::clear_undo_state() {
	before_fill.clear();
}
