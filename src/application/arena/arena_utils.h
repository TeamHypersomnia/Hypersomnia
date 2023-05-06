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
