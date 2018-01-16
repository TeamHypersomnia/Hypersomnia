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

	prefabs::populate_grenade_types(into);
	prefabs::populate_character_types(into);
	prefabs::populate_gun_types(logicals, into);
	prefabs::populate_other_types(logicals, into);
	prefabs::populate_car_types(logicals, into);
	prefabs::populate_crate_types(logicals, into);
	prefabs::populate_melee_types(logicals, into);
	prefabs::populate_backpack_types(logicals, into);
}
