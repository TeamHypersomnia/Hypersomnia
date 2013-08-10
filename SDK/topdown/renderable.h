#pragma once
#include <vector>
#include <Box2D/Box2D.h>

#include "texture_baker/texture_baker.h"
#include "graphics/pixel.h"
#include "math/vec2d.h"

#include "components\transform_component.h"

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

struct renderable {
	static void make_rect(vec2<double> pos, vec2<float> size, float rotation_degrees, vec2<float> out[4]);

	virtual void draw(buffer&, const components::transform&) = 0;
	virtual rects::xywh get_aabb(const components::transform&) = 0;
	virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) = 0;

};

struct sprite : renderable {
	texture_baker::texture* tex;
	graphics::pixel_32 color;
	rects::wh size;

	sprite(texture_baker::texture*, graphics::pixel_32 = graphics::pixel_32());

	virtual void draw(buffer&, const components::transform&) override;
	virtual rects::xywh get_aabb(const components::transform&) override;
	virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) override;
};

struct polygon : renderable {
	std::vector<vertex> vertices;

	virtual void draw(buffer&, const components::transform&) override;
};
