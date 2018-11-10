#pragma once
#include "game/modes/ruleset_id.h"

template <class T>
using make_ruleset_map = std::unordered_map<raw_ruleset_id, typename T::ruleset_type>;

using all_rulesets_map = per_type_container<all_modes, make_ruleset_map>;
