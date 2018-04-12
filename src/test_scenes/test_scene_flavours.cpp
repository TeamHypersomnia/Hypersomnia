#include "game/common_state/entity_flavours.h"
#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/ingredients/ingredients.h"

#include "augs/templates/enum_introspect.h"
#include "augs/string/format_enum.h"

void populate_test_scene_flavours(const loaded_image_caches_map& logicals, all_entity_flavours& into) {
	test_flavours::populate_grenade_flavours(logicals, into);
	test_flavours::populate_character_flavours(logicals, into);
	test_flavours::populate_gun_flavours(logicals, into);
	test_flavours::populate_other_flavours(logicals, into);
	test_flavours::populate_car_flavours(logicals, into);
	test_flavours::populate_crate_flavours(logicals, into);
	test_flavours::populate_melee_flavours(logicals, into);
	test_flavours::populate_backpack_flavours(logicals, into);
}
