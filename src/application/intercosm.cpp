#include "application/intercosm.h"
#include "game/cosmos/cosmic_functions.h"
#include "application/intercosm_io.hpp"

#include "augs/templates/introspection_utils/recursive_clear.h"

#include "game/organization/all_messages_includes.h"
#include "game/cosmos/change_common_significant.hpp"
#include "game/cosmos/change_solvable_significant.h"
#include "game/organization/all_component_includes.h"
#include "view/viewables/image_cache.h"

#include "test_scenes/test_scenes_content.h"

#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"
#include "test_scenes/test_scene_settings.h"

#include "augs/readwrite/lua_file.h"
#include "augs/readwrite/byte_file.h"

#include "game/modes/bomb_defusal.h"
#include "game/modes/test_mode.h"

#include "application/arena/arena_paths.h"
#include "application/predefined_rulesets.h"

static_assert(has_introspect_base_v<const entity_solvable<const controlled_character>>);
static_assert(!augs::is_byte_readwrite_appropriate_v<std::ifstream, all_logical_assets>);
static_assert(augs::is_byte_readwrite_appropriate_v<std::ifstream, augs::simple_pair<int, double>>);

void load_arena_from(
	const arena_paths& paths,
	intercosm& scene,
	predefined_rulesets& rulesets
) {
	scene.load_from_bytes(paths.int_paths);

	try {
		augs::load_from_bytes(rulesets, paths.rulesets_file_path);
	}
	catch (const augs::file_open_error&) {
		/* Just let it happen */
	}
}

void snap_interpolated_to_logical(cosmos&);

void intercosm::clear() {
	cosmic::clear(world);
	viewables.clear();

#if 0
	augs::recursive_clear(version);
	version.commit_number = 0;
#endif
}

void intercosm::populate_official_content(
	sol::state& lua, 
	const unsigned tickrate,
	bomb_defusal_ruleset& bomb_defusal
) {
	clear();

#if !STATICALLY_ALLOCATE_ENTITIES
	cosmic::reserve_storage_for_entities(world, 3000u);
#endif

	const auto caches = populate_test_scene_images_and_sounds(lua, viewables);

	world.change_common_significant([&](cosmos_common_significant& common){
		auto& logicals = common.logical_assets;

		populate_test_scene_logical_assets(viewables.image_definitions, logicals);
		populate_test_scene_viewables(caches, logicals.plain_animations, viewables);
		viewables.update_relevant(logicals);
		::populate_test_scene_common(caches, common);

		return changer_callback_result::REFRESH;
	});

	cosmic::change_solvable_significant(world, [&](auto& s){
		s.clk.now.step = 0;
		s.clk.dt = augs::delta::steps_per_second(tickrate);
		return changer_callback_result::DONT_REFRESH;
	});

	auto populator = test_scenes::testbed();
	populator.setup(bomb_defusal);
	bomb_defusal.speeds.tickrate = tickrate;
}

void intercosm::make_test_scene(
	sol::state& lua, 
	const test_scene_settings settings,
	test_mode_ruleset& test_mode,
	bomb_defusal_ruleset* const bomb_defusal
) {
	clear();

#if BUILD_TEST_SCENES

#if !STATICALLY_ALLOCATE_ENTITIES
	cosmic::reserve_storage_for_entities(world, 3000u);
#endif

	const auto caches = populate_test_scene_images_and_sounds(lua, viewables);

	auto reloader = [&](auto populator) {
		world.change_common_significant([&](cosmos_common_significant& common){
			auto& logicals = common.logical_assets;

			populate_test_scene_logical_assets(viewables.image_definitions, logicals);
			populate_test_scene_viewables(caches, logicals.plain_animations, viewables);
			viewables.update_relevant(logicals);
			::populate_test_scene_common(caches, common);

			return changer_callback_result::REFRESH;
		});

		if (bomb_defusal) {
			bomb_defusal->speeds.tickrate = settings.scene_tickrate;
		}

		populator.populate_with_entities(caches, { world, {}, solve_settings() });

		cosmic::change_solvable_significant(world, [&settings](auto& s){
			/* Populating with test scene will advance it so revert the step number back to 0 */
			s.clk.now.step = 0;
			s.clk.dt = augs::delta::steps_per_second(settings.scene_tickrate);
			return changer_callback_result::DONT_REFRESH;
		});

		populator.setup(test_mode);

		if (bomb_defusal != nullptr) {
			populator.setup(*bomb_defusal);
		}

		snap_interpolated_to_logical(world);
	};

	if (settings.create_minimal) {
		reloader(test_scenes::minimal_scene());
	}
	else {
		reloader(test_scenes::testbed());
	}
#else
	(void)lua;
	(void)settings;
	(void)test_mode;
	(void)bomb_defusal;
#endif
}

void intercosm::save_as_lua(const intercosm_path_op op) const {
	augs::save_as_lua_table(op.lua, *this, op.path);
}

void intercosm::post_load_state_correction() {
	world.change_common_significant([&](cosmos_common_significant& common) {
		/*
			The field:
				all_image_offsets_array_type image_offsets;

			Resides outside of the logical assets' introspection so that it is never written to hdd.
		*/

		viewables.update_relevant(common.logical_assets);

		return changer_callback_result::DONT_REFRESH;
	});

	world.reinfer_everything();
	snap_interpolated_to_logical(world);
	world.request_resample();
}

void intercosm::load_from_lua(const intercosm_path_op op) {
	augs::load_from_lua_table(op.lua, *this, op.path);
}

void intercosm::save_as_bytes(const intercosm_paths& paths) const {
	augs::save_as_bytes(viewables, paths.viewables_file);
	augs::save_as_bytes(world.get_common_significant(), paths.comm_file);
	augs::save_as_bytes(world.get_solvable().significant, paths.solv_file);
}

void intercosm::load_from_bytes(const intercosm_paths& paths) {
	augs::load_from_bytes(viewables, paths.viewables_file);

	world.change_common_significant([&](cosmos_common_significant& common) {
		augs::load_from_bytes(common, paths.comm_file);
		return changer_callback_result::DONT_REFRESH;
	});

	cosmic::change_solvable_significant(world, [&](cosmos_solvable_significant& significant) {
		augs::load_from_bytes(significant, paths.solv_file);
		return changer_callback_result::DONT_REFRESH;
	});

	post_load_state_correction();
}

void intercosm::update_offsets_of(const assets::image_id& id, const changer_callback_result result) {
	world.change_common_significant(
		[&](cosmos_common_significant& common) {
			common.logical_assets.get_offsets(id) = viewables.image_definitions[id].meta.offsets;
			return result;
		}
	);
}

static_assert(augs::has_byte_readwrite_overloads_v<augs::memory_stream, augs::pool<int, make_vector, unsigned>>);
static_assert(augs::has_lua_readwrite_overloads_v<augs::pool<int, of_size<300>::make_nontrivial_constant_vector, unsigned>>);
static_assert(augs::has_lua_readwrite_overloads_v<make_entity_pool<controlled_character>>);