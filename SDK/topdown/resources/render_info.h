#pragma once
#include <vector>
#include <Box2D/Box2D.h>

#include "texture_baker/texture_baker.h"
#include "entity_system/entity_ptr.h"
#include "..\components\transform_component.h"

#include "vertex.h"

using namespace augmentations;

namespace components {
	struct particle_group;
}

namespace resources {
	struct renderable {
		static void make_rect(vec2<> pos, vec2<> size, float rotation_degrees, vec2<> out[4]);

		virtual void draw(buffer&, const components::transform&, vec2<> camera_pos) = 0;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform&) = 0;
		virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) = 0;
	};

	struct sprite : public renderable {
		texture_baker::texture* tex;
		graphics::pixel_32 color;
		vec2<> size;

		sprite(texture_baker::texture* = nullptr, graphics::pixel_32 = graphics::pixel_32());

		void set(texture_baker::texture*, graphics::pixel_32);
		void update_size();

		virtual void draw(buffer&, const components::transform&, vec2<> camera_pos) override;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform&) override;
		virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) override;
	};

	struct polygon : public renderable {
		std::vector<vertex> vertices;

		virtual void draw(buffer&, const components::transform&, vec2<> camera_pos) override;
	};

	struct particles : public renderable {
		components::particle_group* target_group;

		virtual void draw(buffer&, const components::transform&, vec2<> camera_pos) override;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform&) override;
		virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) override;
	};

	struct render_info {
		renderable* model;

		enum mask_type {
			WORLD,
			GUI
		};

		unsigned layer;
		unsigned mask;
	};
}
