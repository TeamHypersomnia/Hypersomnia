#pragma once
#include <variant>
#include <unordered_map>
#include "game/modes/test_scene_mode.h"
#include "augs/templates/per_type.h"

using mode_vars_id = unsigned;

template <class T>
using make_vars_map = std::unordered_map<mode_vars_id, T>;

using all_mode_vars = type_list<
	test_scene_mode_vars
>;

using all_modes_variant = std::variant<
	test_scene_mode
>;

using all_mode_vars_maps = per_type_container<all_mode_vars, make_vars_map>;
