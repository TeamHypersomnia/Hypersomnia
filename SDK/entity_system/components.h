#pragma once
#include "../../entity_system/entity_system.h"
#include "../../rects/rects.h"

using namespace augmentations;
using namespace entity_system;

struct transform_component : public component {
	rects::pointf pos;
	rects::wh size;
	transform_component(rects::pointf pos, rects::wh size) : pos(pos), size(size) {}
};

struct velocity_component : public component {
	rects::pointf vel;
	velocity_component(rects::pointf vel = rects::pointf(0, 0)) : vel(vel) {}
};

struct render_component : public component {
	unsigned r, g, b, a;
	unsigned layer;

	render_component(unsigned layer, unsigned r, unsigned g, unsigned b, unsigned a) 
		: layer(layer), r(r), g(g), b(b), a(a) {}
};

struct input_component : public component {

};

