#include "polygon_component.h"

#include "3rdparty/polypartition/src/polypartition.h"

#include "augs/graphics/renderer.h"

#include "game/assets/assets_manager.h"
#include "game/detail/convex_partitioned_shape.h"

using namespace augs;

namespace components {
	void polygon::set_color(rgba col) {
		for (auto& v : vertices)
			v.color = col;
	}

	void polygon::drawing_input::set_global_time_seconds(const double secs) {
		global_time_seconds = secs;
	}

	void polygon::add_vertices_from(const convex_partitioned_shape& shape) {
		for (const auto& c : shape.convex_polys) {
			std::vector<vertex> new_concave;

			for (auto v : c.vertices) {
				vertex new_v;
				new_v.pos = v;
				new_concave.push_back(new_v);
			}

			add_concave_polygon(new_concave);
		}

		automatically_map_uv(uv_mapping_mode::STRETCH);
	}

	void polygon::automatically_map_uv(const uv_mapping_mode mapping_mode) {
		if (vertices.empty()) {
			return;
		}

		typedef const augs::vertex& vc;

		const auto x_pred = [](const vc a, const vc b) { return a.pos.x < b.pos.x; };
		const auto y_pred = [](const vc a, const vc b) { return a.pos.y < b.pos.y; };

		const auto lower = vec2i(
			static_cast<int>(minimum_of(vertices, x_pred).pos.x),
			static_cast<int>(minimum_of(vertices, y_pred).pos.y)
		);

		const auto upper = vec2i(
			static_cast<int>(maximum_of(vertices, x_pred).pos.x),
			static_cast<int>(maximum_of(vertices, y_pred).pos.y)
		);

		if (mapping_mode == uv_mapping_mode::STRETCH) {
			for (auto& v : vertices) {
				v.texcoord = vec2(
					(v.pos.x - lower.x) / (upper.x - lower.x),
					(v.pos.y - lower.y) / (upper.y - lower.y)
				);
			}
		}
		else {
			ensure(false && "Unknown polygon uv mapping mode!");
		}
	}

	void polygon::add_concave_polygon(std::vector<vertex> polygon) {
		if (polygon.empty()) {
			return;
		}

		size_t i1, i2;

		float area = 0;
		const auto& vs = polygon;

		for (i1 = 0; i1 < vs.size(); i1++) {
			i2 = i1 + 1;
			if (i2 == vs.size()) i2 = 0;
			area += vs[i1].pos.x * vs[i2].pos.y - vs[i1].pos.y * vs[i2].pos.x;
		}

		/* ensure proper winding */
		if (area > 0) {
			std::reverse(polygon.begin(), polygon.end());
		}

		TPPLPoly inpoly;
		std::list<TPPLPoly> out_tris;

		TPPLPoly subject_poly;
		inpoly.Init(polygon.size());
		inpoly.SetHole(false);

		vertices.reserve(vertices.size() + polygon.size());

		int offset = vertices.size();
		for (size_t i = 0; i < polygon.size(); ++i) {
			vertices.push_back(polygon[i]);
			//original_polygon.push_back(polygon[i].pos);
		}

		for (size_t i = 0; i < polygon.size(); ++i) {
			vec2 p(polygon[i].pos);
			inpoly[i].x = p.x;
			inpoly[i].y = -p.y;
		}

		TPPLPartition partition;
		partition.Triangulate_EC(&inpoly, &out_tris);

		for (auto& out : out_tris) {
			for (int i = 0; i < 3; ++i) {
				auto new_tri_point = out.GetPoint(i);

				for (size_t j = offset; j < polygon.size(); ++j) {
					if (polygon[j].pos.compare(vec2(static_cast<float>(new_tri_point.x), static_cast<float>(-new_tri_point.y)), 1.f)) {
						triangulation_indices.push_back(j);
						break;
					}
				}
			}
		}
	}

	std::vector<vec2> polygon::get_vertices() const {
		std::vector<vec2> out;

		for (auto& v : vertices) {
			out.push_back(v.pos);
		}

		return out;
	}
	
	void polygon::draw(const drawing_input& in) const {
		const auto& manager = get_assets_manager();
		const auto& texture = manager.at(texture_map);

		if (in.drawing_type == renderable_drawing_type::NEON_MAPS) {
			components::sprite::drawing_input neon_in(in.target_buffer);
			components::sprite neon_sprite;
			neon_sprite.set(texture_map, texture.get_size());

			neon_in.camera = in.camera;
			neon_in.colorize = in.colorize;
			neon_in.drawing_type = in.drawing_type;
			neon_in.global_time_seconds = in.global_time_seconds;
			neon_in.renderable_transform = in.renderable_transform;
			
			neon_sprite.draw(neon_in);

			return;
		}

		vertex_triangle new_tri;
		const auto camera_pos = in.camera.transform.pos;

		auto model_transformed = vertices;

		if (std::abs(in.renderable_transform.rotation) > 0.f) {
			for (auto& v : model_transformed) {
				v.pos.rotate(in.renderable_transform.rotation, vec2(0, 0));
			}
		}

		/* further rotation of the polygon to fit the camera transform */
		for (auto& v : model_transformed) {
			const auto center = in.camera.visible_world_area / 2;
			
			v.pos += in.renderable_transform.pos - camera_pos + center;

			/* rotate around the center of the screen */
			if (std::abs(in.camera.transform.rotation) > 0.f) {
				v.pos.rotate(in.camera.transform.rotation, center);
			}

			v.pos.x = static_cast<float>(static_cast<int>(v.pos.x));
			v.pos.y = static_cast<float>(static_cast<int>(v.pos.y));

			v.set_texcoord(
				v.texcoord, 
				texture.texture_maps[texture_map_type::DIFFUSE]
			);
		}

		for (size_t i = 0; i < triangulation_indices.size(); i += 3) {
			new_tri.vertices[0] = model_transformed[triangulation_indices[i]];
			new_tri.vertices[1] = model_transformed[triangulation_indices[i + 1]];
			new_tri.vertices[2] = model_transformed[triangulation_indices[i + 2]];

			in.target_buffer.push_back(new_tri);
		}
	}
	
	ltrb polygon::get_aabb(const components::transform transform) const {
		auto model_transformed = vertices;

		for (auto& v : model_transformed) {
			v.pos.rotate(transform.rotation, vec2(0, 0));
			v.pos += transform.pos;
		}

		return augs::get_aabb(
			model_transformed,
			[](const vertex& p) { return p.pos.x; },
			[](const vertex& p) { return p.pos.y; }
		);
	}
}