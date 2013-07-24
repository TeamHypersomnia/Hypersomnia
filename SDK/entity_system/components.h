#pragma once
#include "../../entity_system/entity_system.h"
#include "../../math/rects.h"

using namespace augmentations;
using namespace entity_system;

struct transform_component : public component {
	vec2<float> pos;
	rects::wh size;
	transform_component(vec2<float> pos, rects::wh size) : pos(pos), size(size) {}
};

struct velocity_component : public component {
	vec2<float> vel;
	velocity_component(vec2<float> vel = vec2<float>(0, 0)) : vel(vel) {}
};

struct render_component : public component {
	unsigned r, g, b, a;
	unsigned layer;

	render_component(unsigned layer, unsigned r, unsigned g, unsigned b, unsigned a) 
		: layer(layer), r(r), g(g), b(b), a(a) {}
};

struct input_component : public component {

};

