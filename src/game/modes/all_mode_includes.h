#pragma once
#include <variant>
#include <unordered_map>
#include "game/modes/test_mode.h"
#include "game/modes/bomb_mode.h"

#include "augs/templates/per_type.h"
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/transform_types.h"
#include "augs/templates/list_utils.h"

using all_online_modes = type_list<
	bomb_mode
>;

using rest_of_modes = type_list<
	test_mode
>;

using all_modes = concatenate_lists_t<all_online_modes, rest_of_modes>;

using all_modes_variant = replace_list_type_t<all_modes, std::variant>;
using all_online_modes_variant = replace_list_type_t<all_online_modes, std::variant>;

using mode_type_id = type_in_list_id<all_modes>;
