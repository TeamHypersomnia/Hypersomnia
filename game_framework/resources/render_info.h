#pragma once
#include <vector>
#include "math/vec2.h"

#include "texture_baker/texture_baker.h"
#include "entity_system/entity.h"
#include "..\components\transform_component.h"
#include "..\components\render_component.h"

#include "graphics/vertex.h"

using namespace augs;

class render_system;

namespace components {
	struct particle_group;
	struct render;
}

namespace resources {
	struct renderable {
		struct draw_input {
			render_system* output;
			components::transform::state<> transform, camera_transform;
			components::render* additional_info;
			vec2 visible_area;
			bool always_visible;

			augs::rects::ltrb<float> rotated_camera_aabb;

			draw_input() : output(nullptr), additional_info(nullptr), always_visible(false) {}
		};

		static void make_rect(vec2 pos, vec2 size, float rotation_degrees, vec2 out[4]);

		virtual void draw(draw_input&) = 0;
		virtual std::vector<vec2> get_vertices();
	};

	extern void set_polygon_color(renderable* poly, graphics::pixel_32 col);

	enum uv_mapping_mode {
		OVERLAY,
		STRETCH
	};

	extern void map_texture_to_polygon(renderable* poly, helpers::texture_helper*, unsigned uv_mapping_mode);

	struct sprite : public renderable {
		texture* tex;
		graphics::pixel_32 color;
		vec2 size;
		float rotation_offset;

		sprite(texture* = nullptr, graphics::pixel_32 = graphics::pixel_32());

		void set(texture*, graphics::pixel_32);
		void update_size();

		virtual void draw(draw_input&) override;
		virtual std::vector<vec2> get_vertices() override;
	};

	struct tileset {
		struct tile_type {
			texture* tile_texture;
			tile_type(texture* = nullptr);
		};
		
		std::vector<tile_type> tile_types;
	};

	struct tile_layer : public renderable {
		rects::wh<int> size;
		int square_size = 32;

		tileset* layer_tileset = nullptr;

		struct tile {
			unsigned type_id = 0;

			tile(unsigned type);
		};
		
		tile_layer(rects::wh<int> size);
		
		rects::ltrb<int> get_visible_tiles(draw_input&);
		virtual void draw(draw_input&) override;

		std::vector<std::vector<vec2i>> indices_by_type;
		rects::ltrb<int> indices_by_type_visibility;

		void generate_indices_by_type(rects::ltrb<int>);
		std::vector<tile> tiles;
	};

	//struct triangle : public renderable {
	//	vertex vertices[3];
	//	triangle(const vertex&, const vertex&, const vertex&);
	//
	//	virtual void draw(buffer&, const components::transform::state&, vec2 camera_pos) override;
	//	virtual bool is_visible(rects::xywh visibility_aabb, const components::transform::state&) override;
	//};

	typedef std::vector<vec2> basic_polygon;

	struct polygon : public renderable {
		/* binding facility */
		struct concave {
			std::vector<vertex> vertices;
			void add_vertex(const vertex& v);
		};

		std::vector<vertex> model;

		/* the "model" is already triangulated so we need to preserve original model vertex data if for example 
			we want to form a physics body from this object 
		*/
		basic_polygon original_model;

		std::vector<int> indices;

		int get_vertex_count() const {
			return model.size();
		}

		vertex& get_vertex(int i) {
			return model[i];
		}

		/* construct a set of convex polygons from a potentially concave polygon */
		void add_concave(const concave&);
		//void add_concave_coords(const std::vector<vec2>&);
		//void add_convex(const std::vector<vec2>&);

		virtual void draw(draw_input&) override;
		virtual std::vector<vec2> get_vertices() override;
	};

	struct particles : public renderable {
		components::particle_group* target_group;

	};
}
