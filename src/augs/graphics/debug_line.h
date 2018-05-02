#pragma once
#include <vector>

#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

struct debug_line {
	debug_line() = default;
	debug_line(const rgba col, const vec2 a, const vec2 b) : a(a), b(b), col(col) {}
	debug_line(const vec2 a, const vec2 b, const rgba col) : a(a), b(b), col(col) {}

	vec2 a;
	vec2 b;
	rgba col;
};

using debug_lines = std::vector<debug_line>;