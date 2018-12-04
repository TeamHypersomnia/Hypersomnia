#pragma once

struct intercosm;
struct predefined_rulesets;
struct arena_paths;

void load_arena_from(
	const arena_paths& paths,
	intercosm& scene,
	predefined_rulesets& rulesets
);
