#pragma once
#include "augs/misc/bound.h"
#include "augs/graphics/rgba.h"

#include "game/components/transform_component.h"

struct thunder_input {
	using bound = augs::bound<float>;

	bound delay_between_branches_ms = bound(0.f, 0.f);
	bound max_branch_lifetime_ms = bound(0.f, 0.f);
	bound branch_length = bound(0.f, 0.f);
	unsigned max_depth = 8;
	unsigned max_branch_children = 4;
	unsigned max_all_spawned_branches = 40;

	transformr first_branch_root;
	float branch_angle_spread = 0.f;

	rgba color = cyan;
};