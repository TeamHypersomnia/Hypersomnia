#include "game/cosmos/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/debugger_folder.h"

#include "application/setups/debugger/commands/fill_with_test_scene_command.h"

#include "test_scenes/test_scene_settings.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"
#include "application/setups/debugger/debugger_settings.h"

std::string fill_with_test_scene_command::describe() const {
	return typesafe_sprintf("Filled with %x", minimal ? "minimal test scene" : "test scene");
}

void fill_with_test_scene_command::redo(const debugger_command_input in) {
	/* Reset to zero if it happened while playtesting */
	common.when_happened = 0;

	clear_undo_state();
	in.purge_selections();

	auto& work = in.folder.commanded->work;
	auto& view = in.folder.view;
	auto& view_ids = in.folder.commanded->view_ids;
	auto& player = in.folder.player;
	auto& rulesets = in.folder.commanded->rulesets;

	auto ms = augs::ref_memory_stream(before_fill);

	augs::write_bytes(ms, *in.folder.commanded);
	augs::write_bytes(ms, view);
	augs::write_bytes(ms, player);

	player = {};
	view = {};
	view_ids = {};

	test_mode_ruleset test_ruleset;
	arena_mode_ruleset bomb_ruleset;

#if !IS_PRODUCTION_BUILD
	bomb_ruleset.warmup_secs = 0;
	bomb_ruleset.round_secs = 200;
	bomb_ruleset.freeze_secs = 0;
	bomb_ruleset.round_end_secs = 3;
	bomb_ruleset.max_rounds = 30;
	bomb_ruleset.buy_secs_after_freeze = 60;
	bomb_ruleset.match_summary_seconds = 5;
#endif

	const auto& settings = in.settings.test_scene;

	work.make_test_scene(in.lua, { minimal, settings.scene_tickrate });

	auto& all = rulesets.all;
	all.clear();

	auto& player_id = view.local_player_id;

	const bool pick_arena_mode = settings.start_arena_mode && !minimal;

	{
		const auto test_ruleset_id = raw_ruleset_id(0);
		all.get_for<test_mode>().try_emplace(test_ruleset_id, std::move(test_ruleset));

		if (!pick_arena_mode) {
			const auto arbitrary_player_id = mode_player_id::first();
			player_id = arbitrary_player_id;

			auto& def = rulesets.meta.playtest_default;

			def.type_id.set<test_mode>();
			def.raw = test_ruleset_id;

			rulesets.meta.server_default = def;
		}
	}

	{
		const auto bomb_ruleset_id = raw_ruleset_id(0);
		all.get_for<arena_mode>().try_emplace(bomb_ruleset_id, std::move(bomb_ruleset));

		if (pick_arena_mode) {
			const auto arbitrary_player_id = mode_player_id::first();
			player_id = arbitrary_player_id;

			auto& def = rulesets.meta.playtest_default;

			def.type_id.set<arena_mode>();
			def.raw = bomb_ruleset_id;

			rulesets.meta.server_default = def;
		}
	}
}

void fill_with_test_scene_command::undo(const debugger_command_input in) {
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
