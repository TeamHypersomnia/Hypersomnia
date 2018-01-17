#include "game/common_state/entity_types.h"
#include "test_scenes/test_scene_types.h"
#include "test_scenes/ingredients/ingredients.h"

#include "augs/templates/enum_introspect.h"
#include "augs/templates/format_enum.h"

void populate_test_scene_types(const all_logical_assets& logicals, entity_types& into) {
	into.types.resize(static_cast<std::size_t>(test_scene_type::COUNT));

	augs::for_each_enum([&](const test_scene_type e) {
		auto& new_type = into.types[static_cast<std::size_t>(e)];
		new_type.name = to_wstring(format_enum(e));
	});

	test_types::populate_grenade_types(logicals, into);
	test_types::populate_character_types(logicals, into);
	test_types::populate_gun_types(logicals, into);
	test_types::populate_other_types(logicals, into);
	test_types::populate_car_types(logicals, into);
	test_types::populate_crate_types(logicals, into);
	test_types::populate_melee_types(logicals, into);
	test_types::populate_backpack_types(logicals, into);

	/* Let all renderables have interpolation by default */

	for (auto& t : into.types) {
		if (t.find<definitions::render>()) {
			t.set(definitions::interpolation());
		}
	}
}
