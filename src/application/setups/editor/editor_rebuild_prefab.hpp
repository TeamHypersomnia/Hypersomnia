#pragma once
#include "augs/misc/pool/pool_allocate.h"
#include "augs/templates/traits/has_rotation.h"
#include "augs/templates/traits/has_size.h"
#include "augs/misc/randomization.h"
#include "application/setups/editor/nodes/editor_node_defaults.h"

template <class F>
void editor_setup::rebuild_prefab_nodes(
	const editor_typed_node_id<editor_prefab_node> prefab_node_id,
	F on_created_child,
	const bool call_reverse
) {
	auto prefab_node = find_node(prefab_node_id);

	if (prefab_node == nullptr) {
		return;
	}

	temp_prefab_node_pools.clear();

	auto prefab_resource = find_resource(prefab_node->resource_id);

	if (prefab_resource == nullptr) {
		return;
	}

	using P = editor_builtin_prefab_type;

	const auto name_preffix = prefab_node->get_display_name() + " ";

	auto get_resource_size = [&]<typename R>(R& rid) {
		const auto resource = find_resource(rid);

		if (resource == nullptr) {
			return vec2i::zero;
		}

		return resource->editable.size;
	};

	auto get_node_size = [&]<typename T>(T& node) {
		if constexpr(std::is_same_v<editor_sprite_node, T>) {
			if (node.editable.size.is_enabled) {
				return node.editable.size.value;
			}

			return get_resource_size(node.resource_id);
		}
		else {
			return node.editable.size;
		}
	};

	auto set_color = [&]<typename N>(const rgba col, N* const node) -> N* {
		node->editable.colorize = col; 

		return node;
	};

	auto flip = [&]<typename N>(const bool hori, const bool vert, N* const node) -> N* {
		if (node == nullptr) {
			return nullptr;
		}

		node->editable.flip_horizontally = hori; 
		node->editable.flip_vertically = vert; 

		return node;
	};

	auto align = [&]<typename N>(vec2i dir, N* node) -> N* {
		if (node == nullptr) {
			return nullptr;
		}

		const auto resource = find_resource(node->resource_id);

		if (resource == nullptr) {
			return nullptr;
		}

		auto r = node->editable.rotation;
		const bool swap_dims = r == 90 || r == 270 || r == -90 || r == -270;

		auto hsz = get_node_size(*node) / 2;

		if (swap_dims) {
			std::swap(dir.x, dir.y);
		}

		hsz.x *= dir.x;
		hsz.y *= dir.y;

		if (swap_dims) {
			std::swap(hsz.x, hsz.y);
		}

		node->editable.pos += hsz;

		return node;
	};

	auto create_child = [&](
		auto resource_id,
		transformr offset = transformr(),
		std::optional<vec2> size = std::nullopt,
		const std::string& custom_name = ""
	) -> auto* {
		using R = typename decltype(resource_id)::target_type;
		using N = typename R::node_type;

		const auto resource = find_resource(resource_id);

		if (resource == nullptr) {
			/*
				Because nodes can be null,
				don't ever think of identifying them by indices in the vector of resultant entities.
			*/

			return (N*)nullptr;
		}

		const auto resource_name = resource->get_display_name();

		auto& pool = temp_prefab_node_pools.template get_for<N>();
		pool.emplace_back();
		auto& new_node = pool.back();

		::setup_node_defaults(new_node, *resource);

		new_node.unique_name = get_free_node_name_for(name_preffix + (custom_name.empty() ? resource_name : custom_name));
		new_node.resource_id = resource_id;

		new_node.editable.pos = offset.pos;

		if constexpr(has_rotation_v<decltype(new_node.editable)>) {
			new_node.editable.rotation = offset.rotation;
		}

		if constexpr(has_size_v<decltype(new_node.editable)>) {
			if (size.has_value()) {
				new_node.editable.size = *size;

				if constexpr(std::is_same_v<N, editor_sprite_node>) {
					auto& v = new_node.editable.size.value;
					const auto& rv = resource->editable.size;

					if (v.x <= 0) {
						v.x = rv.x;
					}

					if (v.y <= 0) {
						v.y = rv.y;
					}
				}
			}
		}

		return std::addressof(new_node);
	};

	auto build_aquarium = [&]() {
		const auto& o = official_resource_map;
		const auto& e = prefab_node->editable;
		const auto& a = e.as_aquarium;

		const auto w = e.size.x;
		const auto h = e.size.y;
		const auto w2 = w / 2;
		const auto h2 = h / 2;

		const auto POINT_LIGHT = o[test_static_lights::POINT_LIGHT];

		create_child(o[area_marker_type::ORGANISM_AREA], transformr(), e.size);

		create_child(a.wandering_pixels_1, transformr(), e.size);
		create_child(a.wandering_pixels_2, transformr(), e.size);

		auto sand_size = e.size / 2;
		auto hsand_size = vec2(sand_size) / 2;

		create_child(a.sand_1, -hsand_size, sand_size);
		create_child(a.sand_1, hsand_size,  sand_size);

		create_child(a.sand_2, hsand_size * vec2(1, -1), sand_size);
		create_child(a.sand_2, hsand_size * vec2(-1, 1), sand_size);

		align({1, -1},  create_child(a.sand_edge, vec2(-w2, h2), vec2(w2, 0)));
		align({-1, -1}, create_child(a.sand_edge, vec2(w2, h2),   vec2(w2, 0)));

		//auto dune_sz = get_resource_size(a.dune_big);

		create_child(a.dune_big, vec2(-w2 / 4.5, 0));
		create_child(a.dune_big, vec2(w2 / 3.5, h2 / 2.5));

		//align({0, 1},  create_child(a.wall,  vec2(0, h2),   			vec2(w, 0)));
		align({0, -1}, create_child(a.wall, { vec2(0, -h2), 180 }, 	vec2(w, 0)));

		align({1,  0}, create_child(a.wall, { vec2(w2, 0), -90 },  vec2(h, 0)));
		align({-1, 0}, create_child(a.wall, { vec2(-w2, 0), 90 }, vec2(h, 0)));

		align({-1, 1}, create_child(a.wall_bottom_corners, { vec2(-w2, h2), 0 }));
		align({-1, -1}, create_child(a.wall_top_corners, { vec2(-w2, -h2), 90 }));
		align({1, -1}, create_child(a.wall_top_corners, { vec2(w2, -h2), 180 }));
		align({1, 1}, create_child(a.wall_bottom_corners, { vec2(w2, h2), 270 }));

		align({1, 1}, 					 create_child(a.wall_smooth_end, vec2(-w2, h2)));
		flip(true, false, align({-1, 1}, create_child(a.wall_smooth_end, vec2( w2, h2))));

		{
			auto wse_sz = get_resource_size(a.wall_smooth_end);
			auto gs_sz = get_resource_size(a.glass_start);
			auto gs_off = a.glass_start_offset;

			align(					{1, 1},  create_child(a.glass_start, vec2(-w2 - gs_off, h2) + vec2(wse_sz.x, 0)));
			flip(true, false, align({-1, 1}, create_child(a.glass_start, vec2(w2  + gs_off,  h2) - vec2(wse_sz.x, 0))));

			align({0, 1}, create_child(a.glass, vec2(0, h2), vec2(w - wse_sz.x * 2 - gs_sz.x * 2 + gs_off * 2, 0)));
		}

		align({0, -1}, create_child(a.wall_top_foreground, { vec2(0, -h2), 180 }));

		create_child(a.bubbles, { vec2(0, -h2 + 15), 90 });

		create_child(a.water_overlay, transformr(), e.size);

		if (auto node = create_child(a.collider_interior, transformr(), e.size)) {
			node->editable.colorize.a = 0;
		}

		create_child(a.ambience_left, vec2(-w2, h2));
		create_child(a.ambience_right, vec2(w2, h2));

		{
			constexpr int N = 2;

			rgba wall_lamp_light_cols[N] = {
				rgba(96, 255, 255, 255),
				rgba(103, 255, 69, 255)
			};

			rgba wall_lamp_bodies_cols[N] = {
				rgba(0, 122, 255, 255),
				rgba(0, 180, 59, 255)
			};

			vec2 light_offsets[N] = {
				vec2(29, 0),
				vec2(0, 29)
			};

			std::pair<vec2, transformr> lamp_offsets[N] = {
				{ {1, -1}, {vec2(-w2, h2 - a.wall_lamp_offset), 90} },
				{ {-1, 1}, {vec2(w2 - a.wall_lamp_offset, -h2), 180} }
			};

			for (int hi = 0; hi < N; ++hi) {
				if (auto lamp = create_child(a.wall_lamp_body, lamp_offsets[hi].second)) {
					align(lamp_offsets[hi].first, lamp);

					const auto lamp_tr = lamp->get_transform();

					set_color(wall_lamp_light_cols[hi], create_child(a.wall_lamp_light, lamp_tr));
					set_color(wall_lamp_light_cols[hi], create_child(POINT_LIGHT, lamp_tr + light_offsets[hi]));

					set_color(wall_lamp_bodies_cols[hi], lamp);
				}
			}
		}

		{
			constexpr int N = 1;

			rgba sand_light_cols[N] = {
				rgba(0, 99, 126, 255)
			};

			rgba sand_lamp_light_cols[N] = {
				rgba(96, 255, 255, 255)
			};

			rgba sand_lamp_bodies_cols[N] = {
				rgba(0, 122, 255, 255)
			};

			std::pair<vec2, transformr> lamp_offsets[N] = {
				{ {0, 0}, {vec2(-w2 / 3, -h2 / 3), -45} },
			};

			for (int hi = 0; hi < N; ++hi) {
				if (auto lamp = create_child(a.sand_lamp_body, lamp_offsets[hi].second)) {
					align(lamp_offsets[hi].first, lamp);

					const auto lamp_tr = lamp->get_transform();

					set_color(sand_lamp_light_cols[hi], create_child(a.sand_lamp_light, lamp_tr));
					set_color(sand_light_cols[hi], create_child(POINT_LIGHT, lamp_tr));

					set_color(sand_lamp_bodies_cols[hi], lamp);

					create_child(a.dune_small, lamp_tr.pos + vec2(18, 22));
				}
			}
		}

		{
			const auto flower_1_sz = get_resource_size(a.flower_1);
			const auto flower_2_sz = get_resource_size(a.flower_2);
			//const auto coral_sz = get_resource_size(a.coral);

			std::tuple<vec2, vec2, float> flowers_1[5] = {
				{ vec2(-0.5, -0.3), vec2::zero, 90 },
				{ vec2(-0.5, -0.3), vec2(0, flower_1_sz.x), 90 },
				{ vec2(-0.7, -0.5), vec2::zero, 0 },
				{ vec2(-0.7, -0.5), vec2(flower_1_sz.x, -flower_1_sz.y), 0 },
				{ vec2(-0.3, -0.7), vec2::zero, 0 }
			};

			std::tuple<vec2, vec2, float> flowers_2[3] = {
				{ vec2(0.5, -0.5), vec2::zero, 0 },
				{ vec2(0.5, -0.5), vec2(flower_2_sz.x, -flower_2_sz.y), 0 },
				{ vec2(0.5, -0.5), vec2(0, flower_2_sz.y), 0 }
			};

			std::tuple<vec2, vec2, float> corals[2] = {
				{ vec2(0.6, 0.3), vec2::zero, 0 },
				{ vec2(0.3, 0.6), vec2::zero, 90 }
			};

			auto create_with_offsets = [&](const auto& flavor, const auto& offsets) {
				const auto [ szoff, off, rot ] = offsets;

				const auto final_tr = transformr(szoff * vec2(w2, h2) + off, rot);

				return create_child(flavor, final_tr);
			};

			for (const auto& f : flowers_1) {
				if (auto node = create_with_offsets(a.flower_1, f)) {
					create_child(a.flower_bubbles, node->get_transform());
				}
			}

			for (const auto& f : flowers_2) {
				if (auto node = create_with_offsets(a.flower_2, f)) {
					create_child(a.flower_bubbles, node->get_transform());
				}
			}

			for (const auto& f : corals) {
				create_with_offsets(a.coral, f);
			}
		}

		auto spawn_multiple_of = [&](auto& rng, auto flavour, uint32_t count, auto cb, vec2 bnd_x = vec2(0.0f, 1.0f), vec2 bnd_y = vec2(0.0f, 1.0f)) {
			auto sz = get_resource_size(flavour);

			/* 
				Security measure as someone could forge a map that requests infinite amounts
			*/

			count = std::min(count, 50u);

			for (uint32_t i = 0; i < count; ++i) {
				auto rand_x = rng.randval(bnd_x.x, bnd_x.y);
				auto rand_y = rng.randval(bnd_y.x, bnd_y.y);

				auto bigger_dim = sz.bigger_side();
				auto min_x = -w2 + bigger_dim;
				auto min_y = -h2 + bigger_dim;
				auto max_x = w2 - bigger_dim;
				auto max_y = h2 - bigger_dim;

				const auto x = augs::interp(min_x, max_x, rand_x);
				const auto y = augs::interp(min_y, max_y, rand_y);

				const auto final_tr = transformr(vec2(x, y), rng.randval(0, 360));
				cb(create_child(flavour, final_tr));
			}
		};

		{
			auto rng = randomization(a.fish_seed);

			auto cb = [](auto...){};
			spawn_multiple_of(rng, a.fish_1, a.fish_1_count, cb);
			spawn_multiple_of(rng, a.fish_2, a.fish_2_count, cb);
			spawn_multiple_of(rng, a.fish_3, a.fish_3_count, cb);
			spawn_multiple_of(rng, a.fish_4, a.fish_4_count, cb);
			spawn_multiple_of(rng, a.fish_5, a.fish_5_count, cb);
			spawn_multiple_of(rng, a.fish_6, a.fish_6_count, cb);
		}

		{
			auto rng = randomization(a.caustics_seed);

			spawn_multiple_of(rng, a.caustics, a.caustics_count / 3, [&](auto* node) {
				if (node) {
					node->editable.starting_animation_frame.emplace(rng.randval(0, 100));
				}
			}, vec2(0.0, 0.5), vec2(0.0, 0.5));

			spawn_multiple_of(rng, a.caustics, a.caustics_count / 3, [&](auto* node) {
				if (node) {
					node->editable.starting_animation_frame.emplace(rng.randval(0, 100));
				}
			}, vec2(0.0, 0.5), vec2(0.5, 1.0));

			const auto rmdr = a.caustics_count - 2 * (a.caustics_count / 3);

			spawn_multiple_of(rng, a.caustics, rmdr, [&](auto* node) {
				if (node) {
					node->editable.starting_animation_frame.emplace(rng.randval(0, 100));
				}
			}, vec2(0.5, 1.0), vec2(0.0, 0.5));
		}

		{
			auto rng = randomization(a.dim_caustics_seed);

			spawn_multiple_of(rng, a.caustics, a.dim_caustics_count, [&](auto* node) {
				if (node) {
					node->editable.starting_animation_frame.emplace(rng.randval(0, 100));
					node->editable.colorize.a = 79;
				}
			}, vec2(0.5, 1.0), vec2(0.5, 1.0));
		}
	};

	switch (prefab_resource->editable.type) {
		case P::AQUARIUM: build_aquarium(); break;
		default: break;
	}

	auto callback = [&]<typename N>(N& child_node) {
		const auto final_transform = prefab_node->get_transform() * child_node.get_transform();

		child_node.editable.pos = final_transform.pos;

		if constexpr(has_rotation_v<decltype(child_node.editable)>) {
			child_node.editable.rotation = final_transform.rotation;
		}

		on_created_child(child_node);
	};

	if (call_reverse) {
		temp_prefab_node_pools.for_each_reverse(callback);
	}
	else {
		temp_prefab_node_pools.for_each(callback);
	}
}
