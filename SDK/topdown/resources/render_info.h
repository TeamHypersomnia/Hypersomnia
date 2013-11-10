#pragma once
#include <vector>

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

		virtual void draw(buffer&, const components::transform::state&, vec2<> camera_pos) = 0;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform::state&) = 0;
		virtual std::vector<vec2<>> get_vertices();
	};

	struct sprite : public renderable {
		texture_baker::texture* tex;
		graphics::pixel_32 color;
		vec2<> size;

		sprite(texture_baker::texture* = nullptr, graphics::pixel_32 = graphics::pixel_32());

		void set(texture_baker::texture*, graphics::pixel_32);
		void update_size();

		virtual void draw(buffer&, const components::transform::state&, vec2<> camera_pos) override;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform::state&) override;
		virtual std::vector<vec2<>> get_vertices() override;
	};

	//struct triangle : public renderable {
	//	vertex vertices[3];
	//	triangle(const vertex&, const vertex&, const vertex&);
	//
	//	virtual void draw(buffer&, const components::transform::state&, vec2<> camera_pos) override;
	//	virtual bool is_visible(rects::xywh visibility_aabb, const components::transform::state&) override;
	//};

	struct polygon : public renderable {
		/* binding facility */
		struct concave {
			std::vector<vertex> vertices;
			void add_vertex(const vertex& v);
		};

		std::vector<vertex> model;
		std::vector<int> indices;

		//void add_convex(const std::vector<vertex>&);
		void add_concave(const concave&);
		virtual void draw(buffer&, const components::transform::state&, vec2<> camera_pos) override;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform::state&) override;
		virtual std::vector<vec2<>> get_vertices() override;
	};

	struct particles : public renderable {
		components::particle_group* target_group;

	};
}
