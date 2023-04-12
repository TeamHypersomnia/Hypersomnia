#pragma once
#include "application/arena/mode_and_rules.h"

namespace sol {
	class state;
}

struct intercosm;
struct predefined_rulesets;
struct arena_paths;

void load_intercosm_and_rulesets(
	const arena_paths& paths,
	intercosm& scene,
	predefined_rulesets& rulesets
);

void make_test_online_arena(
	sol::state& lua,
	intercosm& scene,
	online_mode_and_rules&,
	predefined_rulesets& rulesets
);
