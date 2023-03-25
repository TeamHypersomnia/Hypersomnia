#pragma once
#include "game/modes/ruleset_id.h"

template <class T>
using make_ruleset_map = std::unordered_map<raw_ruleset_id, T>;

template <class T>
using make_ruleset_map_from_mode = make_ruleset_map<typename T::ruleset_type>;

using all_rulesets_map = per_type_container<all_modes, make_ruleset_map_from_mode>;
