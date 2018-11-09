#pragma once
#include <variant>
#include <unordered_map>
#include "game/modes/test_mode.h"
#include "game/modes/bomb_mode.h"

#include "augs/templates/per_type.h"
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/transform_types.h"

using raw_ruleset_id = unsigned;

template <class T>
using make_ruleset_map = std::unordered_map<raw_ruleset_id, typename T::ruleset_type>;

using all_modes = type_list<
	test_mode,
	bomb_mode
>;

using mode_type_id = type_in_list_id<all_modes>;

using all_rulesets_map = per_type_container<all_modes, make_ruleset_map>;
using all_modes_variant = replace_list_type_t<all_modes, std::variant>;
