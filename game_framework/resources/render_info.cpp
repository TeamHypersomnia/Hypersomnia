#include "render_info.h"

#include <algorithm>
#include "entity_system/entity.h"

#include "../components/physics_component.h"
#include "../components/particle_group_component.h"
#include "../systems/render_system.h"

#include "misc/sorted_vector.h"

#include "3rdparty/polypartition/polypartition.h"

namespace resources {
	void set_polygon_color(renderable* poly, graphics::pixel_32 col) {
		polygon* p = (polygon*) poly;

		for (auto& v : p->model) {
			v.color = col;
		}
	}

	void map_texture_to_polygon(renderable* poly, helpers::texture_helper* texture_to_map, unsigned uv_mapping_mode) {
		polygon* p = (polygon*) poly;

		if (p->model.empty()) return;

		auto* v = p->model.data();
		typedef const augs::vertex& vc;

		auto x_pred = [](vc a, vc b){ return a.pos.x < b.pos.x; };
		auto y_pred = [](vc a, vc b){ return a.pos.y < b.pos.y; };

		vec2i lower(
			static_cast<int>(std::min_element(v, v + p->model.size(), x_pred)->pos.x),
			static_cast<int>(std::min_element(v, v + p->model.size(), y_pred)->pos.y)
			);

		vec2i upper(
			static_cast<int>(std::max_element(v, v + p->model.size(), x_pred)->pos.x),
			static_cast<int>(std::max_element(v, v + p->model.size(), y_pred)->pos.y)
			);

		if (uv_mapping_mode == uv_mapping_mode::STRETCH) {
			for (auto& v : p->model) {
				v.set_texcoord(vec2(
					(v.pos.x - lower.x) / (upper.x - lower.x),
					(v.pos.y - lower.y) / (upper.y - lower.y)
					), texture_to_map);
			}
		}
		else if (uv_mapping_mode == uv_mapping_mode::OVERLAY) {
			auto size = texture_to_map->get_size();

			for (auto& v : p->model) {
				v.set_texcoord(vec2(
					(v.pos.x - lower.x) / size.x,
					(v.pos.y - lower.y) / size.y
					), texture_to_map);
			}
		}
	}

	void renderable::make_rect(vec2 pos, vec2 size, float angle, vec2 v[4]) {
		vec2 origin(pos + (size / 2.f));

		v[0] = pos;
		v[1] = pos + vec2(size.x, 0.f);
		v[2] = pos + size;
		v[3] = pos + vec2(0.f, size.y);

		v[0].rotate(angle, origin);
		v[1].rotate(angle, origin);
		v[2].rotate(angle, origin);
		v[3].rotate(angle, origin);

		v[0] -= size / 2.f;
		v[1] -= size / 2.f;
		v[2] -= size / 2.f;
		v[3] -= size / 2.f;
	}

	std::vector<vec2> renderable::get_vertices() {
		return std::vector<vec2>();
	}

	sprite::sprite(texture* tex, graphics::pixel_32 color) : tex(tex), color(color), rotation_offset(0.f) {
		set(tex, color);
	}

	void sprite::set(texture* _tex, graphics::pixel_32 _color) {
		tex = _tex;
		color = _color;

		if (tex)
			size = tex->get_size();
	}

	void sprite::update_size() {
		if (tex)
			size = tex->get_size();
	}

	void sprite::draw(draw_input& in) {
		if (in.additional_info) {
			in.additional_info->was_drawn = false;
		}

		vec2 v[4];
		vec2i transform_pos = in.transform.pos;
		make_rect(transform_pos, vec2(size), in.transform.rotation, v);
		if (!in.always_visible && !rects::ltrb<float>::get_aabb(v).hover(in.rotated_camera_aabb)) return;

		if (tex == nullptr) return;

		auto center = in.visible_area / 2;

		auto target_position = transform_pos - in.camera_transform.pos + center;
		make_rect(target_position, vec2(size), in.transform.rotation + rotation_offset, v);

		/* rotate around the center of the screen */
		if(std::abs(in.camera_transform.rotation) > 0)
			for (auto& vert : v)
				vert.rotate(in.camera_transform.rotation, center);

		vertex_triangle t1, t2;
		t1.vertices[0].color = t2.vertices[0].color = color;
		t1.vertices[1].color = t2.vertices[1].color = color;
		t1.vertices[2].color = t2.vertices[2].color = color;

		vec2 texcoords[] = {
			vec2(0.f, 0.f),
			vec2(1.f, 0.f),
			vec2(1.f, 1.f),
			vec2(0.f, 1.f)
		};

		if (in.additional_info) {
			if (in.additional_info->flip_horizontally)
				for (auto& v : texcoords)
					v.x = 1.f - v.x; 
				if (in.additional_info->flip_vertically)
				for (auto& v : texcoords)
					v.y = 1.f - v.y;
		}

		t1.vertices[0].texcoord = t2.vertices[0].texcoord = texcoords[0];
		t2.vertices[1].texcoord =							texcoords[1];
		t1.vertices[1].texcoord = t2.vertices[2].texcoord = texcoords[2];
		t1.vertices[2].texcoord =							texcoords[3];

		for (int i = 0; i < 3; ++i) {
			tex->get_uv(t1.vertices[i].texcoord);
			tex->get_uv(t2.vertices[i].texcoord);
		}

		t1.vertices[0].pos = t2.vertices[0].pos = vec2i(v[0]);
		t2.vertices[1].pos = vec2i(v[1]);
		t1.vertices[1].pos = t2.vertices[2].pos = vec2i(v[2]);
		t1.vertices[2].pos = vec2i(v[3]);

		if (in.additional_info) {
			/* compute average */
			in.additional_info->last_screen_pos = (vec2(v[0]) + vec2(v[1]) + vec2(v[2]) + vec2(v[3])) / 4;
			in.additional_info->was_drawn = true;
		}

		in.output->push_triangle(t1);
		in.output->push_triangle(t2);
	}

	std::vector<vec2> sprite::get_vertices() {
		std::vector<vec2> out;
		out.push_back(size / -2.f);
		out.push_back(size / -2.f + vec2(size.x, 0.f));
		out.push_back(size / -2.f + size);
		out.push_back(size / -2.f + vec2(0.f, size.y));
		return std::move(out);
	}

	//triangle::triangle(const vertex& a, const vertex& b, const vertex& c) {
	//	vertices[0] = a;
	//	vertices[1] = b;
	//	vertices[2] = c;
	//}
	//
	//void triangle::draw(buffer& triangles, const components::transform::state& transform, vec2 camera_pos) {
	//
	//}

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

	void polygon::draw(draw_input& in) {
		vertex_triangle new_tri;
		auto camera_pos = in.camera_transform.pos;

		std::vector<int> visible_indices;
		
		auto model_transformed = model;

		/* initial transformation for visibility checks */
		if(std::abs(in.transform.rotation) > 0.f)
			for (auto& v : model_transformed)
				v.pos.rotate(in.transform.rotation, vec2(0, 0));
		
		if (in.always_visible) {
			visible_indices = indices;
		}
		else {
			/* visibility checking every triangle */
			for (size_t i = 0; i < indices.size(); i += 3) {
				new_tri.vertices[0] = model_transformed[indices[i]];
				new_tri.vertices[1] = model_transformed[indices[i + 1]];
				new_tri.vertices[2] = model_transformed[indices[i + 2]];

				new_tri.vertices[0].pos += in.transform.pos;
				new_tri.vertices[1].pos += in.transform.pos;
				new_tri.vertices[2].pos += in.transform.pos;

				auto* v = new_tri.vertices;
				typedef const augs::vertex& vc;

				auto x_pred = [](vc a, vc b){ return a.pos.x < b.pos.x; };
				auto y_pred = [](vc a, vc b){ return a.pos.y < b.pos.y; };

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
			v.pos += in.transform.pos - camera_pos + center;

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

	tile_layer::tile_layer(rects::wh<int> size) : size(size) {
		tiles.reserve(size.area());
	}

	tile_layer::tile::tile(unsigned type) : type_id(type) {}
	tileset::tile_type::tile_type(texture* tile_texture) : tile_texture(tile_texture) {

	}

	rects::ltrb<int> tile_layer::get_visible_tiles(draw_input& in) {
		rects::ltrb<int> visible_tiles;

		visible_tiles.l = int((in.rotated_camera_aabb.l - in.transform.pos.x) / 32.f);
		visible_tiles.t = int((in.rotated_camera_aabb.t - in.transform.pos.y) / 32.f);
		visible_tiles.r = int((in.rotated_camera_aabb.r - in.transform.pos.x) / 32.f) + 1;
		visible_tiles.b = int((in.rotated_camera_aabb.b - in.transform.pos.y) / 32.f) + 1;
		visible_tiles.l = std::max(0, visible_tiles.l);
		visible_tiles.t = std::max(0, visible_tiles.t);
		visible_tiles.r = std::min(size.w, visible_tiles.r);
		visible_tiles.b = std::min(size.h, visible_tiles.b);

		return visible_tiles;
	}

	void tile_layer::draw(draw_input& in) {
		/* if it is not visible, return */
		if (!in.rotated_camera_aabb.hover(rects::xywh<float>(in.transform.pos.x, in.transform.pos.y, size.w*square_size, size.h*square_size))) return;

		auto visible_tiles = get_visible_tiles(in);

		draw_input draw_input_copy = in;
		
		for (int y = visible_tiles.t; y < visible_tiles.b; ++y) {
			for (int x = visible_tiles.l; x < visible_tiles.r; ++x) {
				vertex_triangle t1, t2;
				
				auto tile_offset = vec2i(x, y) * square_size;
				
				int idx = y * size.w + x;
				
				if (tiles[idx].type_id == 0) continue;
				
				auto& type = layer_tileset->tile_types[tiles[idx].type_id-1];

				sprite tile_sprite(type.tile_texture);
				draw_input_copy.transform.pos = vec2i(in.transform.pos) + tile_offset + vec2(square_size / 2, square_size/2);
				
				tile_sprite.draw(draw_input_copy);
			}
		}
	}

	void tile_layer::generate_indices_by_type(rects::ltrb<int> visible_tiles) {
		if (visible_tiles == indices_by_type_visibility)
			return;

		indices_by_type_visibility = visible_tiles;
		
		for (auto& index_vector : indices_by_type)
			index_vector.clear();

		for (int y = visible_tiles.t; y < visible_tiles.b; ++y) {
			for (int x = visible_tiles.l; x < visible_tiles.r; ++x) {
				int i = y * size.w + x;

				auto type = tiles[i].type_id;
				if (type == 0) continue;

				if (indices_by_type.size() < type + 1)
					indices_by_type.resize(type + 1);

				indices_by_type[type].push_back(vec2i(x, y) * square_size);
			}
		}
	}
}

namespace components {
	void particle_group::draw(draw_input& in) {
		for (auto& s : stream_slots)
			for (auto& it : s.particles.particles) {
				auto temp_alpha = it.face.color.a;

				if (it.should_disappear) {
					auto desired_alpha = static_cast<graphics::color>(((it.max_lifetime_ms - it.lifetime_ms) / it.max_lifetime_ms) * static_cast<float>(temp_alpha));
					if (it.alpha_levels > 0) {
						it.face.color.a = desired_alpha == 0 ? 0 : ((255 / it.alpha_levels) * (1 + (desired_alpha / (255 / it.alpha_levels))));
					}
					else {
						it.face.color.a = desired_alpha;
					}
				}

				in.transform = it.ignore_rotation ? components::transform::state<>(it.pos, 0) : components::transform::state<>(it.pos, it.rotation);
				it.face.draw(in);
				it.face.color.a = temp_alpha;
			}
	}
}
