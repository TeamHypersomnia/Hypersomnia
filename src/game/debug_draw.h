#pragma once
#include <cstddef>
#include <vector>
#include "augs/math/declare_math.h"
#include "augs/graphics/debug_line.h"

class b2PolygonShape;
struct si_scaling;

void debug_draw_shape(std::vector<debug_line>&, rgba, const b2PolygonShape&, vec2 pos, si_scaling);
void debug_draw_shape(std::vector<debug_line>&, rgba, const b2PolygonShape&, vec2 pos);

template <std::size_t I>
void debug_draw_verts(std::vector<debug_line>& lines, const rgba col, const std::array<vec2, I>& arr, const vec2 pos);
