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

#include "game/modes/bomb_mode.h"
#include "game/modes/test_mode.h"

static_assert(has_introspect_base_v<const entity_solvable<const controlled_character>>);
static_assert(!augs::is_byte_readwrite_appropriate_v<std::ifstream, all_logical_assets>);
static_assert(augs::is_byte_readwrite_appropriate_v<std::ifstream, augs::simple_pair<int, double>>);

void intercosm::clear() {
	cosmic::clear(world);
	viewables.clear();

#if 0
	augs::recursive_clear(version);
	version.commit_number = 0;
#endif
}

#if BUILD_TEST_SCENES
void intercosm::make_test_scene(
	sol::state& lua, 
	const test_scene_settings settings,
	test_mode_ruleset& test_mode,
	bomb_mode_ruleset* const bomb_mode
) {
	clear();

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

		if (bomb_mode) {
			bomb_mode->speeds.tickrate = settings.scene_tickrate;
		}

		populator.populate_with_entities(caches, { world, {} });

		cosmic::change_solvable_significant(world, [&settings](auto& s){
			/* Populating with test scene will advance it so revert the step number back to 0 */
			s.clk.now.step = 0;
			s.clk.dt = augs::delta::steps_per_second(settings.scene_tickrate);
			return changer_callback_result::DONT_REFRESH;
		});

		populator.setup(test_mode);

		if (bomb_mode != nullptr) {
			populator.setup(*bomb_mode);
		}
	};

	if (settings.create_minimal) {
		reloader(test_scenes::minimal_scene());
	}
	else {
		reloader(test_scenes::testbed());
	}
}
#endif

void intercosm::save(const intercosm_path_op op) const {
	const auto effective_extension = op.path.extension();

	if (effective_extension == ".int") {
		save_as_int(op.path);
	}
	else if (effective_extension == ".lua") {
		save_as_lua(op);
	}
}

void intercosm::load(const intercosm_path_op op) {
	const auto effective_extension = op.path.extension();

	if (effective_extension == ".int") {
		load_from_int(op.path);
	}
	else if (effective_extension == ".lua") {
		load_from_lua(op);
	}
}

void intercosm::save_as_lua(const intercosm_path_op op) const {
	augs::save_as_lua_table(op.lua, *this, op.path);
}

void intercosm::post_load_state_correction() {
	world.change_common_significant([&](cosmos_common_significant& common) {
		(void)common;

		/*
			The field:
				all_image_offsets_array_type image_offsets;

			Resides outside of the logical assets' introspection so that it is never written to hdd.
		*/

		viewables.update_relevant(common.logical_assets);

		return changer_callback_result::DONT_REFRESH;
	});

	world.reinfer_everything();
}

void intercosm::load_from_lua(const intercosm_path_op op) {
	augs::load_from_lua_table(op.lua, *this, op.path);
}

void intercosm::save_as_int(const augs::path_type& path) const {
	augs::save_as_bytes(*this, path);
}

void intercosm::load_from_int(const augs::path_type& path) {
	augs::load_from_bytes(*this, path);
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
static_assert(augs::has_lua_readwrite_overloads_v<augs::pool<int, of_size<300>::make_constant_vector, unsigned>>);
static_assert(augs::has_lua_readwrite_overloads_v<make_entity_pool<controlled_character>>);