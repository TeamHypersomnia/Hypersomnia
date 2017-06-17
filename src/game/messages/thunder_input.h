#pragma once
#include "game/components/transform_component.h"
#include "augs/graphics/rgba.h"

struct thunder_input {
	typedef augs::minmax<float> minmax;

	minmax delay_between_branches_ms = minmax(0.f, 0.f);
	minmax max_branch_lifetime_ms = minmax(0.f, 0.f);
	minmax branch_length = minmax(0.f, 0.f);
	unsigned max_depth = 8;
	unsigned max_branch_children = 4;
	unsigned max_all_spawned_branches = 40;

	components::transform first_branch_root;
	float branch_angle_spread = 0.f;

	rgba color;
};