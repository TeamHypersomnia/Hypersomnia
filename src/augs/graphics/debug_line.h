#pragma once
#include <array>
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

/*
	debug_rect stores 4 corners for drawing a filled quad.
	Corners should be in order: top-left, top-right, bottom-right, bottom-left.
*/

struct debug_rect {
	debug_rect() = default;
	debug_rect(const rgba col, const std::array<vec2, 4>& corners) : corners(corners), col(col) {}
	debug_rect(const std::array<vec2, 4>& corners, const rgba col) : corners(corners), col(col) {}

	std::array<vec2, 4> corners;
	rgba col;
};

using debug_rects = std::vector<debug_rect>;