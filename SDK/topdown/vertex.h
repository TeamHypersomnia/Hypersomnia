#pragma once
#include "graphics/pixel.h"
#include "math/vec2d.h"

using namespace augmentations;

struct vertex {
	vec2<int> position;
	vec2<float> texcoord;
	graphics::pixel_32 color;
};

struct triangle {
	vertex vertices[3];
};

typedef std::vector<triangle> buffer;
