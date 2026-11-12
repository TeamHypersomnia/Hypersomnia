#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/drawing/sprite.h"
#include "augs/drawing/drawing_input_base.h"
#include "augs/graphics/dedicated_buffers.h"
#include "augs/graphics/shader.h"
#include "augs/graphics/common_uniform_name.h"
#include "game/components/sprite_component.h"
#include "game/components/star_field_component.h"
#include "view/rendering_scripts/draw_entity_input.h"

inline constexpr std::size_t star_field_types_vec4_count_declared_in_illuminated_vsh_v = 48;
inline constexpr std::size_t vec4s_per_star_field_type_v = 3;

static_assert(
	MAX_STAR_FIELD_STAR_TYPES * vec4s_per_star_field_type_v
	== star_field_types_vec4_count_declared_in_illuminated_vsh_v
);

inline constexpr real32 star_field_cell_size_v = 512.f;
inline constexpr real32 largest_star_field_coordinate_castable_to_cell_v = 1000000000.f;
inline constexpr uint32_t max_star_field_quads_v = 400000;

inline real32 calc_star_field_spin_degrees(const invariants::sprite& field_sprite, const double global_time_seconds) {
	if (field_sprite.effect != augs::sprite_special_effect::CONTINUOUS_ROTATION) {
		return 0.f;
	}

	return static_cast<real32>(std::fmod(global_time_seconds * field_sprite.effect_speed_multiplier * 360.0, 360.0));
}

inline real32 calc_star_field_total_weight(const invariants::star_field& field) {
	auto total_weight = 0.f;

	for (const auto& type : field.star_types) {
		total_weight += type.weight;
	}

	return total_weight;
}

struct star_field_view {
	vec2 center;
	real32 rotation_cos = 1.f;
	real32 rotation_sin = 0.f;
	ltrb drawn_area;
	vec2i first_cell;
	vec2i last_cell;
	int whole_stars_per_cell = 0;
	real32 chance_of_one_more_star = 0.f;
	real32 total_weight = 0.f;
	rgba color = white;

	bool is_empty() const {
		return drawn_area.l >= drawn_area.r || drawn_area.t >= drawn_area.b || total_weight <= 0.f;
	}

	int calc_slots_per_cell() const {
		return whole_stars_per_cell + (chance_of_one_more_star > 0.f ? 1 : 0);
	}

	vec2i calc_cell_count() const {
		return last_cell - first_cell + vec2i(1, 1);
	}
};

inline star_field_view calc_star_field_view(
	const invariants::star_field& field,
	const invariants::sprite& field_sprite,
	const sprite_drawing_input& field_input,
	const ltrb visible_world,
	const real32 largest_star_half_diagonal
) {
	auto view = star_field_view();

	view.center = field_input.renderable_transform.pos;

	const auto spin_degrees = ::calc_star_field_spin_degrees(field_sprite, field_input.global_time_seconds);
	const auto rotation_degrees = field_input.renderable_transform.rotation + spin_degrees;
	const auto radians = rotation_degrees * DEG_TO_RAD<real32>;
	view.rotation_cos = static_cast<real32>(std::cos(radians));
	view.rotation_sin = static_cast<real32>(std::sin(radians));

	const auto field_half_size = vec2(field_sprite.get_size()) / 2;

	const vec2 visible_corners[4] = {
		visible_world.lt(),
		visible_world.rt(),
		visible_world.rb(),
		visible_world.lb()
	};

	auto to_local = [&](const vec2 world) {
		const auto offset = world - view.center;
		return vec2(
			offset.x * view.rotation_cos + offset.y * view.rotation_sin,
			-offset.x * view.rotation_sin + offset.y * view.rotation_cos
		);
	};

	auto visible_lt = to_local(visible_corners[0]);
	auto visible_rb = visible_lt;

	for (const auto corner : visible_corners) {
		const auto local = to_local(corner);
		visible_lt = vec2(std::min(visible_lt.x, local.x), std::min(visible_lt.y, local.y));
		visible_rb = vec2(std::max(visible_rb.x, local.x), std::max(visible_rb.y, local.y));
	}

	const auto margin = largest_star_half_diagonal;

	view.drawn_area = ltrb(
		std::max(visible_lt.x - margin, -field_half_size.x),
		std::max(visible_lt.y - margin, -field_half_size.y),
		std::min(visible_rb.x + margin, field_half_size.x),
		std::min(visible_rb.y + margin, field_half_size.y)
	);

	const bool area_castable_to_cells =
		std::abs(view.drawn_area.l) < largest_star_field_coordinate_castable_to_cell_v &&
		std::abs(view.drawn_area.t) < largest_star_field_coordinate_castable_to_cell_v &&
		std::abs(view.drawn_area.r) < largest_star_field_coordinate_castable_to_cell_v &&
		std::abs(view.drawn_area.b) < largest_star_field_coordinate_castable_to_cell_v
	;

	if (!area_castable_to_cells) {
		return star_field_view();
	}

	const auto cell_size = star_field_cell_size_v;

	auto cell_of = [cell_size](const vec2 local) {
		return vec2i(
			static_cast<int>(std::floor(local.x / cell_size)),
			static_cast<int>(std::floor(local.y / cell_size))
		);
	};

	view.first_cell = cell_of(view.drawn_area.lt());
	view.last_cell = cell_of(view.drawn_area.rb());

	const auto requested_stars_per_cell = field.stars_per_million_square_units * cell_size * cell_size / 1000000.f;
	const auto stars_per_cell =
		std::isfinite(requested_stars_per_cell) ?
		std::clamp(requested_stars_per_cell, 0.f, static_cast<real32>(max_star_field_quads_v)) :
		0.f
	;
	view.whole_stars_per_cell = static_cast<int>(stars_per_cell);
	view.chance_of_one_more_star = stars_per_cell - static_cast<real32>(view.whole_stars_per_cell);
	view.total_weight = ::calc_star_field_total_weight(field);

	view.color = field_sprite.color;

	if (field_input.colorize != white) {
		view.color *= field_input.colorize;
	}

	return view;
}

struct star_field_uniform_locations {
	GLint mode = -1;
	GLint transform = -1;
	GLint area = -1;
	GLint cells = -1;
	GLint density = -1;
	GLint look = -1;
	GLint color = -1;
	GLint seed = -1;
	GLint types = -1;

	static star_field_uniform_locations of(const augs::graphics::shader_program& program) {
		using U = augs::common_uniform_name;

		auto result = star_field_uniform_locations();

		result.mode = program.get_common_uniform_location(U::star_field_mode);
		result.transform = program.get_common_uniform_location(U::star_field_transform);
		result.area = program.get_common_uniform_location(U::star_field_area);
		result.cells = program.get_common_uniform_location(U::star_field_cells);
		result.density = program.get_common_uniform_location(U::star_field_density);
		result.look = program.get_common_uniform_location(U::star_field_look);
		result.color = program.get_common_uniform_location(U::star_field_color);
		result.seed = program.get_common_uniform_location(U::star_field_seed);
		result.types = program.get_common_uniform_location(U::star_field_types);

		return result;
	}

	bool all_found() const {
		const GLint all[] = { mode, transform, area, cells, density, look, color, seed, types };

		for (const auto location : all) {
			if (location == -1) {
				return false;
			}
		}

		return true;
	}
};

struct star_field_gpu_target {
	augs::triangles_and_specials* buffer = nullptr;
	star_field_uniform_locations locations;
	ltrb visible_world_unscaled_by_query_mult;

	bool is_usable() const {
		return buffer != nullptr && locations.all_found();
	}
};

template <class M>
std::vector<vec4> make_star_field_type_table(const invariants::star_field& field, const M& manager) {
	std::vector<vec4> table;
	table.reserve(field.star_types.size() * vec4s_per_star_field_type_v);

	auto cumulative_weight = 0.f;

	for (const auto& type : field.star_types) {
		const auto& entry = manager.at(type.image_id);
		const auto& diffuse = entry.diffuse;
		const auto size = vec2(entry.get_original_size());

		const auto uv0 = diffuse.get_atlas_space_uv(vec2(0.f, 0.f));
		const auto uv1 = diffuse.get_atlas_space_uv(vec2(1.f, 0.f));
		const auto uv2 = diffuse.get_atlas_space_uv(vec2(1.f, 1.f));
		const auto uv3 = diffuse.get_atlas_space_uv(vec2(0.f, 1.f));

		cumulative_weight += type.weight;

		table.push_back({ uv0.x, uv0.y, uv1.x, uv1.y });
		table.push_back({ uv2.x, uv2.y, uv3.x, uv3.y });
		table.push_back({ size.x, size.y, cumulative_weight, 0.f });
	}

	return table;
}

template <class M>
real32 calc_largest_star_half_diagonal(const invariants::star_field& field, const M& manager) {
	auto largest_diagonal = 0.f;

	for (const auto& type : field.star_types) {
		const auto size = vec2(manager.at(type.image_id).get_original_size());
		largest_diagonal = std::max(largest_diagonal, size.length());
	}

	const auto largest_scale = std::max(std::abs(field.min_scale), std::abs(field.max_scale));

	return largest_diagonal * largest_scale / 2;
}

template <class M>
void queue_star_field_on_gpu(
	star_field_gpu_target& target,
	const invariants::star_field& field,
	const invariants::sprite& field_sprite,
	const sprite_drawing_input& field_input,
	const M& manager
) {
	if (!target.is_usable()) {
		return;
	}

	const auto view = ::calc_star_field_view(
		field,
		field_sprite,
		field_input,
		target.visible_world_unscaled_by_query_mult,
		::calc_largest_star_half_diagonal(field, manager)
	);

	if (view.is_empty()) {
		return;
	}

	const auto cell_count = view.calc_cell_count();
	const auto slots_per_cell = view.calc_slots_per_cell();
	const auto quad_count =
		static_cast<uint64_t>(cell_count.x)
		* static_cast<uint64_t>(cell_count.y)
		* static_cast<uint64_t>(slots_per_cell)
	;

	if (quad_count == 0 || quad_count > max_star_field_quads_v) {
		return;
	}

	const auto& locations = target.locations;
	auto& buffer = *target.buffer;

	auto draw = augs::procedural_quads_draw();

	draw.after_triangles = static_cast<uint32_t>(buffer.triangles.size());
	draw.quad_count = static_cast<uint32_t>(quad_count);

	draw.uniforms_before = {
		{ locations.mode, 1 },
		{ locations.transform, vec4 { view.center.x, view.center.y, view.rotation_cos, view.rotation_sin } },
		{ locations.area, vec4 { view.drawn_area.l, view.drawn_area.t, view.drawn_area.r, view.drawn_area.b } },
		{ locations.cells, vec4 {
			static_cast<float>(view.first_cell.x),
			static_cast<float>(view.first_cell.y),
			static_cast<float>(cell_count.x),
			static_cast<float>(slots_per_cell)
		} },
		{ locations.density, vec4 {
			static_cast<float>(view.whole_stars_per_cell),
			view.chance_of_one_more_star,
			star_field_cell_size_v,
			static_cast<float>(field.star_types.size())
		} },
		{ locations.look, vec4 { field.min_scale, field.max_scale, field.min_brightness, view.total_weight } },
		{ locations.color, view.color },
		{ locations.seed, static_cast<int>(field.seed) }
	};

	draw.uniforms_after = {
		{ locations.mode, 0 }
	};

	draw.vec4_array_uniform_id = locations.types;
	draw.vec4_array = ::make_star_field_type_table(field, manager);

	buffer.procedural_quads.push_back(std::move(draw));
}

inline draw_renderable_input with_star_fields_on_gpu(const draw_renderable_input& in, star_field_gpu_target* const target) {
	return draw_renderable_input {
		{
			in.drawer,
			in.manager,
			in.global_time_seconds,
			in.real_global_time_seconds,
			in.flip,
			in.randomizing,
			in.cone,
			in.draw_long_bullet_neons,
			in.bullets_alpha,
			target
		},
		in.interp,
		in.decals
	};
}
