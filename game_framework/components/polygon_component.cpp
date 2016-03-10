#include "polygon_component.h"
#include "texture_baker/texture_baker.h"

#include "3rdparty/polypartition/polypartition.h"

#include "graphics/renderer.h"
#include "../detail/state_for_drawing.h"

#include "game_framework/resources/manager.h"

using namespace augs;

#include <cassert>

namespace components {
	void polygon::set_color(rgba col) {
		for (auto& v : triangulated_polygon)
			v.color = col;
	}

	void polygon::automatically_map_uv(assets::texture_id texture_id_to_map, unsigned uv_mapping_mode) {
		if (triangulated_polygon.empty()) return;

		auto& texture_to_map = resource_manager.find(texture_id_to_map)->tex;

		auto* v = triangulated_polygon.data();
		typedef const augs::vertex& vc;

		auto x_pred = [](vc a, vc b) { return a.pos.x < b.pos.x; };
		auto y_pred = [](vc a, vc b) { return a.pos.y < b.pos.y; };

		vec2i lower(
			static_cast<int>(std::min_element(v, v + triangulated_polygon.size(), x_pred)->pos.x),
			static_cast<int>(std::min_element(v, v + triangulated_polygon.size(), y_pred)->pos.y)
			);

		vec2i upper(
			static_cast<int>(std::max_element(v, v + triangulated_polygon.size(), x_pred)->pos.x),
			static_cast<int>(std::max_element(v, v + triangulated_polygon.size(), y_pred)->pos.y)
			);

		if (uv_mapping_mode == uv_mapping_mode::STRETCH) {
			for (auto& v : triangulated_polygon) {
				v.set_texcoord(vec2(
					(v.pos.x - lower.x) / (upper.x - lower.x),
					(v.pos.y - lower.y) / (upper.y - lower.y)
					), texture_to_map);
			}
		}
		else if (uv_mapping_mode == uv_mapping_mode::OVERLAY) {
			auto size = texture_to_map.get_size();

			for (auto& v : triangulated_polygon) {
				v.set_texcoord(vec2(
					(v.pos.x - lower.x) / size.x,
					(v.pos.y - lower.y) / size.y
					), texture_to_map);
			}
		}
	}

	void polygon::add_polygon_vertices(std::vector<vertex> polygon) {
		if (polygon.empty()) return;
		size_t i1, i2;

		float area = 0;
		auto& vs = polygon;

		for (i1 = 0; i1 < vs.size(); i1++) {
			i2 = i1 + 1;
			if (i2 == vs.size()) i2 = 0;
			area += vs[i1].pos.x * vs[i2].pos.y - vs[i1].pos.y * vs[i2].pos.x;
		}

		/* ensure proper winding */
		if (area > 0) std::reverse(polygon.begin(), polygon.end());


		TPPLPoly inpoly;
		list<TPPLPoly> out_tris;

		TPPLPoly subject_poly;
		inpoly.Init(polygon.size());
		inpoly.SetHole(false);

		triangulated_polygon.reserve(triangulated_polygon.size() + polygon.size());

		int offset = triangulated_polygon.size();
		for (size_t i = 0; i < polygon.size(); ++i) {
			triangulated_polygon.push_back(polygon[i]);
			original_polygon.push_back(polygon[i].pos);
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
					if (polygon[j].pos.compare(vec2(new_tri_point.x, -new_tri_point.y), 1.f)) {
						indices.push_back(j);
						break;
					}
				}
			}
		}
	}

	std::vector<vec2> polygon::get_vertices() {
		std::vector<vec2> out;

		for (auto& v : triangulated_polygon)
			out.push_back(v.pos);

		return std::move(out);
	}

	void polygon::draw(const shared::state_for_drawing_renderable& in) const {
		vertex_triangle new_tri;
		auto camera_pos = in.camera_transform.pos;

		auto model_transformed = triangulated_polygon;

		/* initial transformation for visibility checks */
		if (std::abs(in.renderable_transform.rotation) > 0.f)
			for (auto& v : model_transformed)
				v.pos.rotate(in.renderable_transform.rotation, vec2(0, 0));

		/* further rotation of the polygon to fit the camera transform */
		for (auto& v : model_transformed) {
			auto center = in.visible_world_area / 2;
			v.pos += in.renderable_transform.pos - camera_pos + center;

			/* rotate around the center of the screen */
			if (std::abs(in.camera_transform.rotation) > 0.f)
				v.pos.rotate(in.camera_transform.rotation, center);

			v.pos.x = int(v.pos.x);
			v.pos.y = int(v.pos.y);
		}

		for (size_t i = 0; i < indices.size(); i += 3) {
			new_tri.vertices[0] = model_transformed[indices[i]];
			new_tri.vertices[1] = model_transformed[indices[i + 1]];
			new_tri.vertices[2] = model_transformed[indices[i + 2]];

			in.output->push_triangle(new_tri);
		}
	}
	
	rects::ltrb<float> polygon::get_aabb(components::transform transform) {
		return rects::ltrb<float>::get_aabb(original_polygon);
	}
}