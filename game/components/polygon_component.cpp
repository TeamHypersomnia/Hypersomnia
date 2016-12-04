#include "polygon_component.h"
#include "augs/texture_baker/texture_baker.h"

#include "3rdparty/polypartition/src/polypartition.h"

#include "augs/graphics/renderer.h"
#include "game/detail/state_for_drawing_camera.h"

#include "game/resources/manager.h"

using namespace augs;

namespace components {
	void polygon::set_color(rgba col) {
		for (auto& v : vertices)
			v.color = col;
	}

	void polygon::drawing_input::set_global_time_seconds(const float secs) {
		global_time_seconds = secs;
	}

	void polygon::automatically_map_uv(assets::texture_id texture_id_to_map, unsigned uv_mapping_mode) {
		if (vertices.empty()) return;

		auto& texture_to_map = get_resource_manager().find(texture_id_to_map)->tex;

		auto* v = vertices.data();
		typedef const augs::vertex& vc;

		auto x_pred = [](vc a, vc b) { return a.pos.x < b.pos.x; };
		auto y_pred = [](vc a, vc b) { return a.pos.y < b.pos.y; };

		vec2i lower(
			static_cast<int>(std::min_element(v, v + vertices.size(), x_pred)->pos.x),
			static_cast<int>(std::min_element(v, v + vertices.size(), y_pred)->pos.y)
			);

		vec2i upper(
			static_cast<int>(std::max_element(v, v + vertices.size(), x_pred)->pos.x),
			static_cast<int>(std::max_element(v, v + vertices.size(), y_pred)->pos.y)
			);

		if (uv_mapping_mode == uv_mapping_mode::STRETCH) {
			for (auto& v : vertices) {
				v.set_texcoord(vec2(
					(v.pos.x - lower.x) / (upper.x - lower.x),
					(v.pos.y - lower.y) / (upper.y - lower.y)
					), texture_to_map);
			}
		}
		else if (uv_mapping_mode == uv_mapping_mode::OVERLAY) {
			auto size = texture_to_map.get_size();

			for (auto& v : vertices) {
				v.set_texcoord(vec2(
					(v.pos.x - lower.x) / size.x,
					(v.pos.y - lower.y) / size.y
					), texture_to_map);
			}
		}
	}

	void polygon::add_concave_polygon(std::vector<vertex> polygon) {
		if (polygon.empty()) return;
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

		for (auto& v : vertices)
			out.push_back(v.pos);

		return std::move(out);
	}
	
	void polygon::draw(const drawing_input& in) const {
		vertex_triangle new_tri;
		const auto camera_pos = in.camera.transform.pos;

		auto model_transformed = vertices;

		/* initial transformation for visibility checks */
		if (std::abs(in.renderable_transform.rotation) > 0.f)
			for (auto& v : model_transformed)
				v.pos.rotate(in.renderable_transform.rotation, vec2(0, 0));

		/* further rotation of the polygon to fit the camera transform */
		for (auto& v : model_transformed) {
			auto center = in.camera.visible_world_area / 2;
			v.pos += in.renderable_transform.pos - camera_pos + center;

			/* rotate around the center of the screen */
			if (std::abs(in.camera.transform.rotation) > 0.f)
				v.pos.rotate(in.camera.transform.rotation, center);

			v.pos.x = static_cast<float>(static_cast<int>(v.pos.x));
			v.pos.y = static_cast<float>(static_cast<int>(v.pos.y));
		}

		for (size_t i = 0; i < triangulation_indices.size(); i += 3) {
			new_tri.vertices[0] = model_transformed[triangulation_indices[i]];
			new_tri.vertices[1] = model_transformed[triangulation_indices[i + 1]];
			new_tri.vertices[2] = model_transformed[triangulation_indices[i + 2]];

			in.target_buffer.push_back(new_tri);
		}
	}
	
	rects::ltrb<float> polygon::get_aabb(components::transform transform) const {
		return augs::get_aabb(vertices, 
			[](const vertex p) { return p.pos.x; },
			[](const vertex p) { return p.pos.y; }
		);
	}
}