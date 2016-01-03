#include "polygon_component.h"
#include "texture_baker/texture_baker.h"

#include "3rdparty/polypartition/polypartition.h"

#include "graphics/renderer.h"
#include "../shared/drawing_state.h"

#include "game_framework/resources/manager.h"

using namespace augs;

namespace components {
	void polygon::set_color(pixel_32 col) {
		for (auto& v : model)
			v.color = col;
	}

	void polygon::automatically_map_uv(assets::texture_id texture_id_to_map, unsigned uv_mapping_mode) {
		if (model.empty()) return;

		auto& texture_to_map = resource_manager.find(texture_id_to_map)->tex;

		auto* v = model.data();
		typedef const augs::vertex& vc;

		auto x_pred = [](vc a, vc b) { return a.pos.x < b.pos.x; };
		auto y_pred = [](vc a, vc b) { return a.pos.y < b.pos.y; };

		vec2i lower(
			static_cast<int>(std::min_element(v, v + model.size(), x_pred)->pos.x),
			static_cast<int>(std::min_element(v, v + model.size(), y_pred)->pos.y)
			);

		vec2i upper(
			static_cast<int>(std::max_element(v, v + model.size(), x_pred)->pos.x),
			static_cast<int>(std::max_element(v, v + model.size(), y_pred)->pos.y)
			);

		if (uv_mapping_mode == uv_mapping_mode::STRETCH) {
			for (auto& v : model) {
				v.set_texcoord(vec2(
					(v.pos.x - lower.x) / (upper.x - lower.x),
					(v.pos.y - lower.y) / (upper.y - lower.y)
					), texture_to_map);
			}
		}
		else if (uv_mapping_mode == uv_mapping_mode::OVERLAY) {
			auto size = texture_to_map.get_size();

			for (auto& v : model) {
				v.set_texcoord(vec2(
					(v.pos.x - lower.x) / size.x,
					(v.pos.y - lower.y) / size.y
					), texture_to_map);
			}
		}
	}


	void polygon::concave::add_vertex(const vertex& v) {
		vertices.push_back(v);
	}

	void polygon::add_concave(const concave& original_polygon) {
		if (original_polygon.vertices.empty()) return;
		size_t i1, i2;

		auto polygon = original_polygon;

		float area = 0;
		auto& vs = polygon.vertices;

		for (i1 = 0; i1 < vs.size(); i1++) {
			i2 = i1 + 1;
			if (i2 == vs.size()) i2 = 0;
			area += vs[i1].pos.x * vs[i2].pos.y - vs[i1].pos.y * vs[i2].pos.x;
		}

		/* ensure proper winding */
		if (area > 0) std::reverse(polygon.vertices.begin(), polygon.vertices.end());


		TPPLPoly inpoly;
		list<TPPLPoly> out_tris;

		TPPLPoly subject_poly;
		inpoly.Init(polygon.vertices.size());
		inpoly.SetHole(false);

		model.reserve(model.size() + polygon.vertices.size());

		int offset = model.size();
		for (size_t i = 0; i < polygon.vertices.size(); ++i) {
			model.push_back(polygon.vertices[i]);
			original_model.push_back(polygon.vertices[i].pos);
		}

		for (size_t i = 0; i < polygon.vertices.size(); ++i) {
			vec2 p(polygon.vertices[i].pos);
			inpoly[i].x = p.x;
			inpoly[i].y = -p.y;
		}

		TPPLPartition partition;
		partition.Triangulate_EC(&inpoly, &out_tris);

		for (auto& out : out_tris) {
			for (int i = 0; i < 3; ++i) {
				auto new_tri_point = out.GetPoint(i);

				for (size_t j = offset; j < polygon.vertices.size(); ++j) {
					if (polygon.vertices[j].pos.compare(vec2(new_tri_point.x, -new_tri_point.y), 1.f)) {
						indices.push_back(j);
						break;
					}
				}
			}
		}
	}

	std::vector<vec2> polygon::get_vertices() {
		std::vector<vec2> out;

		for (auto& v : model)
			out.push_back(v.pos);

		return std::move(out);
	}

	//void polygon::add_convex(const std::vector<vertex>& model) {
	//	convex_models.push_back(model);
	//}

	void polygon::draw(shared::drawing_state& in) {
		vertex_triangle new_tri;
		auto camera_pos = in.camera_transform.pos;

		std::vector<int> visible_indices;

		auto model_transformed = model;

		/* initial transformation for visibility checks */
		if (std::abs(in.drawn_transform.rotation) > 0.f)
			for (auto& v : model_transformed)
				v.pos.rotate(in.drawn_transform.rotation, vec2(0, 0));

		if (in.always_visible) {
			visible_indices = indices;
		}
		else {
			/* visibility checking every triangle */
			for (size_t i = 0; i < indices.size(); i += 3) {
				new_tri.vertices[0] = model_transformed[indices[i]];
				new_tri.vertices[1] = model_transformed[indices[i + 1]];
				new_tri.vertices[2] = model_transformed[indices[i + 2]];

				new_tri.vertices[0].pos += in.drawn_transform.pos;
				new_tri.vertices[1].pos += in.drawn_transform.pos;
				new_tri.vertices[2].pos += in.drawn_transform.pos;

				auto* v = new_tri.vertices;
				typedef const augs::vertex& vc;

				auto x_pred = [](vc a, vc b) { return a.pos.x < b.pos.x; };
				auto y_pred = [](vc a, vc b) { return a.pos.y < b.pos.y; };

				vec2i lower(
					static_cast<int>(std::min_element(v, v + 3, x_pred)->pos.x),
					static_cast<int>(std::min_element(v, v + 3, y_pred)->pos.y)
					);

				vec2i upper(
					static_cast<int>(std::max_element(v, v + 3, x_pred)->pos.x),
					static_cast<int>(std::max_element(v, v + 3, y_pred)->pos.y)
					);

				/* only if the triangle is visible should we render the indices */
				if (rects::ltrb<float>(lower.x, lower.y, upper.x, upper.y).hover(in.rotated_camera_aabb)) {
					visible_indices.push_back(indices[i]);
					visible_indices.push_back(indices[i + 1]);
					visible_indices.push_back(indices[i + 2]);
				}
			}
		}

		/* further rotation of the polygon to fit the camera transform */
		for (auto& v : model_transformed) {
			auto center = in.visible_area / 2;
			v.pos += in.drawn_transform.pos - camera_pos + center;

			/* rotate around the center of the screen */
			if (std::abs(in.camera_transform.rotation) > 0.f)
				v.pos.rotate(in.camera_transform.rotation, center);

			v.pos.x = int(v.pos.x);
			v.pos.y = int(v.pos.y);
		}

		for (size_t i = 0; i < visible_indices.size(); i += 3) {
			new_tri.vertices[0] = model_transformed[visible_indices[i]];
			new_tri.vertices[1] = model_transformed[visible_indices[i + 1]];
			new_tri.vertices[2] = model_transformed[visible_indices[i + 2]];

			in.output->push_triangle(new_tri);
		}
	}
}