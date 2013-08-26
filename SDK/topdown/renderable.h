#pragma once
#include <vector>
#include <Box2D/Box2D.h>

#include "texture_baker/texture_baker.h"
#include "entity_system/entity_ptr.h"
#include "components\transform_component.h"

#include "vertex.h"
using namespace augmentations;
struct renderable {
	static void make_rect(vec2<> pos, vec2<> size, float rotation_degrees, vec2<> out[4]);

	virtual void draw(buffer&, const components::transform&) = 0;
	virtual bool is_visible(rects::xywh visibility_aabb, const components::transform&) = 0;
	virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) = 0;

};

struct sprite : public renderable {
	texture_baker::texture* tex;
	graphics::pixel_32 color;
	vec2<int> size;

	sprite(texture_baker::texture*, graphics::pixel_32 = graphics::pixel_32());

	virtual void draw(buffer&, const components::transform&) override;
	virtual bool is_visible(rects::xywh visibility_aabb, const components::transform&) override;
	virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) override;
};

struct polygon : public renderable {
	std::vector<vertex> vertices;

	virtual void draw(buffer&, const components::transform&) override;
};

namespace components {
	struct particle_group;
}

struct particles_renderable : public renderable {
	augmentations::entity_system::entity_ptr particles;

	particles_renderable(augmentations::entity_system::entity*);
	virtual void draw(buffer&, const components::transform&) override;
	virtual bool is_visible(rects::xywh visibility_aabb, const components::transform&) override;
	virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) override;
};