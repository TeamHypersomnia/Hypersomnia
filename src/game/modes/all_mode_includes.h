#pragma once
#include <variant>
#include <unordered_map>
#include "game/modes/test_scene_mode.h"
#include "game/modes/bomb_mode.h"

#include "augs/templates/per_type.h"
#include "augs/templates/type_in_list_id.h"

using mode_vars_id = unsigned;

template <class T>
using make_vars_map = std::unordered_map<mode_vars_id, T>;

using all_mode_vars = type_list<
	test_scene_mode_vars,
	bomb_mode_vars
>;

using mode_vars_type_id = type_in_list_id<all_mode_vars>;

using all_modes_variant = std::variant<
	test_scene_mode,
	bomb_mode
>;

using all_mode_vars_maps = per_type_container<all_mode_vars, make_vars_map>;
