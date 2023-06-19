#ifdef BUILD_DEBUGGER_SETUP
#define BUILD_INTERCOSM_IO 1
#elif DEBUG_DESYNCS
#define BUILD_INTERCOSM_IO 1
#else
#define BUILD_INTERCOSM_IO 0
#endif

#include "application/intercosm.h"
#include "game/cosmos/cosmic_functions.h"

#include "game/organization/all_messages_includes.h"
#include "game/cosmos/change_common_significant.hpp"
#include "game/cosmos/change_solvable_significant.h"
#include "game/organization/all_component_includes.h"
#include "view/viewables/image_cache.h"

#include "test_scenes/test_scenes_content.h"

#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"
#include "test_scenes/test_scene_settings.h"

#if BUILD_INTERCOSM_IO
#include "application/intercosm_io.hpp"

#include "augs/readwrite/lua_file.h"
#include "augs/readwrite/byte_file.h"
#include "application/arena/arena_paths.h"
#endif

#include "game/modes/arena_mode.h"
#include "game/modes/test_mode.h"

void snap_interpolated_to_logical(cosmos&);

void intercosm::clear() {
	cosmic::clear(world);
	viewables.clear();
}

void intercosm::populate_official_content(
	sol::state& lua, 
	const unsigned tickrate
) {
	clear();

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
}

void intercosm::make_test_scene(
	sol::state& lua, 
	const test_scene_settings settings
) {
	clear();

#if BUILD_TEST_SCENES
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

		auto entropy = cosmic_entropy();
		populator.populate_with_entities(caches, { world, entropy, solve_settings() });

		cosmic::change_solvable_significant(world, [&settings](auto& s){
			/* Populating with test scene will advance it so revert the step number back to 0 */
			s.clk.now.step = 0;
			s.clk.dt = augs::delta::steps_per_second(settings.scene_tickrate);
			return changer_callback_result::DONT_REFRESH;
		});

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
#endif
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


void intercosm::update_offsets_of(const assets::image_id& id, const changer_callback_result result) {
	world.change_common_significant(
		[&](cosmos_common_significant& common) {
			common.logical_assets.get_offsets(id) = viewables.image_definitions[id].meta.offsets;
			return result;
		}
	);
}

#if BUILD_INTERCOSM_IO
void intercosm::save_as_lua(const intercosm_path_op op) const {
	augs::save_as_lua_table(op.lua, *this, op.path);
}

void intercosm::load_from_lua(const intercosm_path_op op) {
	augs::load_from_lua_table(op.lua, *this, op.path);
}

#if DEBUG_DESYNCS
bool LOG_BYTE_SERIALIZE = false;
#endif

void intercosm::save_as_bytes(const intercosm_paths& paths) const {
	augs::save_as_bytes(viewables, paths.viewables_file);
	augs::save_as_bytes(world.get_common_significant(), paths.comm_file);
#if DEBUG_DESYNCS
	LOG_BYTE_SERIALIZE = true;
#endif

	augs::save_as_bytes(world.get_solvable().significant, paths.solv_file);

#if DEBUG_DESYNCS
	LOG_BYTE_SERIALIZE = false;
#endif
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

static_assert(has_introspect_base_v<const entity_solvable<const controlled_character>>);
static_assert(!augs::is_byte_readwrite_appropriate_v<std::ifstream, all_logical_assets>);
static_assert(augs::is_byte_readwrite_appropriate_v<std::ifstream, augs::simple_pair<int, double>>);

static_assert(augs::has_byte_readwrite_overloads_v<augs::memory_stream, augs::pool<int, make_vector, unsigned>>);
static_assert(augs::has_lua_readwrite_overloads_v<augs::pool<int, of_size<300>::make_nontrivial_constant_vector, unsigned>>);
static_assert(augs::has_lua_readwrite_overloads_v<make_entity_pool<controlled_character>>);

#endif