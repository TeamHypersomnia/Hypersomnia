#pragma once
#include <variant>
#include <unordered_map>
#include "game/modes/test_mode.h"
#include "game/modes/bomb_defusal.h"

#include "augs/templates/per_type.h"
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/transform_types.h"
#include "augs/templates/list_utils.h"

using all_modes = type_list<
	test_mode,
	bomb_defusal
>;

using all_modes_variant = replace_list_type_t<all_modes, std::variant>;
using mode_type_id = type_in_list_id<all_modes>;
