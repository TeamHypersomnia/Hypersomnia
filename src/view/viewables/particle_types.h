#pragma once
#include <cstddef>
#include <limits>
#include "augs/drawing/drawing.h"
#include "augs/math/vec2.h"
#include "view/viewables/particle_types_declaration.h"
#include "augs/graphics/rgba.h"
#include "game/cosmos/entity_id.h"

#include "game/detail/view_input/particle_effect_modifier.h"
#include "game/assets/all_logical_assets.h"
#include "game/assets/asset_pools.h"
#include "game/assets/animation.h"

#include "augs/drawing/sprite_helpers.h"
#include "game/detail/view_input/explosion_particles_def.h"

template <class T, class = void>
struct has_lifetime : std::false_type {};

template <class T>
struct has_lifetime<T, decltype(std::declval<T&>().current_lifetime_ms, void())> : std::true_type {};

template <class T>
constexpr bool has_lifetime_v = has_lifetime<T>::value;

struct general_particle {
	static constexpr std::size_t statically_allocate = 10000;

	// GEN INTROSPECTOR struct general_particle
	vec2 pos;
	vec2 vel;
	vec2 acc;
	assets::image_id image_id;
	rgba color = white;
	vec2i size;
	float rotation = 0.f;
	float rotation_speed = 0.f;
	float linear_damping = 0.f;
	float angular_damping = 0.f;
	float current_lifetime_ms = 0.f;
	float max_lifetime_ms = 0.f;
	float shrink_when_ms_remaining = 0.f;
	float unshrinking_time_ms = 0.f;
	rgba start_color = rgba(0, 0, 0, 0);
	float start_color_fade_ms = 0.f;

	bool smooth_shrink = false;
	// END GEN INTROSPECTOR

	/*
		Runtime only. Bullet trail particles slow down together with the logic speed.
	*/

	float time_mult = 1.f;

	void integrate(const float dt);

	template <bool use_neon_maps, class M>
	void draw_as_sprite(
		augs::vertex_triangle& t1,
		augs::vertex_triangle& t2,
		const M& manager,
		const plain_animations_pool&
	) const {
		float size_mult = 1.f;

		if (shrink_when_ms_remaining > 0.f) {
			const auto alivity_multiplier = std::min(1.f, (max_lifetime_ms - current_lifetime_ms) / shrink_when_ms_remaining);

			size_mult *= std::sqrt(alivity_multiplier);
		}

		if (unshrinking_time_ms > 0.f) {
			size_mult *= std::min(1.f, (current_lifetime_ms / unshrinking_time_ms)*(current_lifetime_ms / unshrinking_time_ms));
		}

		/*
			When start_color_fade_ms > 0, the particle spawns with start_color
			and blends into its target color over the first start_color_fade_ms of its lifetime.
		*/
		auto considered_color = color;

		if (start_color_fade_ms > 0.f && current_lifetime_ms < start_color_fade_ms) {
			considered_color = augs::interp(start_color, color, current_lifetime_ms / start_color_fade_ms);
		}

		auto draw = [&](const vec2 drawn_size) {
			if constexpr(use_neon_maps) {
				augs::detail_write_neon_sprite(t1, t2, manager.at(image_id), drawn_size, pos, rotation, considered_color);
			}
			else {
				augs::detail_write_sprite(t1, t2, manager.at(image_id), drawn_size, pos, rotation, considered_color);
			}
		};

		if (size_mult != 1.f) {
			const auto scaled_size = vec2(size) * size_mult;

			/*
				Snapping the shrinking size to whole pixels is the norm (e.g. blood particles
				rely on it - with so many of them shrinking at once, the abrupt per-pixel steps
				read as an intentional "pop" rather than a glitch). Some particles (e.g. the thin
				sniper trace dashes) look better with a smooth, fractional-pixel fade instead.
			*/
			if (smooth_shrink) {
				if (scaled_size.area() > 1.f) {
					draw(scaled_size);
				}
			}
			else {
				if (const auto target_size = vec2i(scaled_size); target_size.area() > 1) {
					draw(vec2(target_size));
				}
			}
		}
		else {
			draw(size);
		}
	}

	bool is_dead() const;

	void set_position(const vec2);
	void set_velocity(const vec2);
	void set_acceleration(const vec2);
	void multiply_size(const float);
	void set_rotation(const float);
	void set_rotation_speed(const float);
	void set_max_lifetime_ms(const float);
	void colorize(const rgba);

	void set_image(assets::image_id, vec2i size, rgba);
};

/*
	A general_particle moving in polar coordinates around a fixed center,
	for the pixel art explosions approximating the exploding rings.

	The sprite always lies tangent to the circle it is on (plus rotation_offset).
	tangential_vel is in pixels per second, so the angular velocity follows the current radius.
	The drift is an independent cartesian offset, driven by drift_acc.

	The particle dies upon reaching max_radius (e.g. a wall that occluded the explosion)
	or crossing the center.

	Both velocities decay exponentially by velocity_damping (per second) for an explosive ease-out.

	The color goes from hot_color through ring_color to cool_color, by the time since the spawn -
	all the particles of one explosion spawn at once, so they all change color in sync.
	Always fully opaque.

	A non-empty palette replaces all that: the color is interpolated between the palette's entries
	at palette_offset + (time since the spawn / palette_step_ms), clamped to the last one.
*/

struct explosion_particle {
	general_particle sprite;

	vec2 center;
	vec2 drift;
	vec2 drift_vel;
	vec2 drift_acc;

	float radius = 0.f;
	float radial_vel = 0.f;
	float tangential_vel = 0.f;
	float angle = 0.f;
	float rotation_offset = 0.f;
	float max_radius = std::numeric_limits<float>::max();

	float velocity_damping = 0.f;

	rgba hot_color = white;
	rgba ring_color = white;
	rgba cool_color = white;
	float hot_until_ms = 0.f;
	float cool_from_ms = std::numeric_limits<float>::max();
	float cool_until_ms = std::numeric_limits<float>::max();

	explosion_particles_palette palette;
	float palette_offset = 0.f;
	float palette_step_ms = 1.f;

	void integrate(const float dt);
	void update_sprite_color();
	void update_sprite_transform();

	template <bool use_neon_maps, class M>
	void draw_as_sprite(
		augs::vertex_triangle& t1,
		augs::vertex_triangle& t2,
		const M& manager,
		const plain_animations_pool& anims
	) const {
		sprite.template draw_as_sprite<use_neon_maps>(t1, t2, manager, anims);
	}

	bool is_dead() const {
		return sprite.is_dead();
	}
};

struct animation_in_particle {
	// GEN INTROSPECTOR struct animation_in_particle
	float speed_factor = 1.f;

	assets::plain_animation_id id;
	simple_animation_state state;
	// END GEN INTROSPECTOR

	void advance(const real32 dt, const plain_animations_pool& anims) {
		if (const auto found_animation = mapped_or_nullptr(anims, id)) {
			if (state.advance(dt * speed_factor, found_animation->frames)) {
				speed_factor = -1.f;
			}

			return;
		}

		speed_factor = -1.f;
	}

	auto get_image_id(const plain_animations_pool& anims) const {
		if (const auto found_animation = mapped_or_nullptr(anims, id)) {
			return found_animation->get_image_id(state);
		}

		return assets::image_id();
	}

	bool is_dead() const {
		return speed_factor <= 0.f;
	}

	bool should_integrate(const plain_animations_pool& anims) const {
		if (const auto found_animation = mapped_or_nullptr(anims, id)) {
			const auto& stop = found_animation->meta.stop_movement_at_frame;

			if (stop.is_enabled) {
				if (state.frame_num >= stop.value) {
					return false;
				}
			}

			return true;
		}

		return false;
	}
};

struct animated_particle {
	static constexpr std::size_t statically_allocate = 10000;

	// GEN INTROSPECTOR struct animated_particle
	vec2 pos;
	vec2 vel;
	vec2 acc;
	animation_in_particle animation;
	float linear_damping = 0.f;

	rgba color = white;
	// END GEN INTROSPECTOR

	/*
		Runtime only. Bullet trail particles slow down together with the logic speed.
	*/

	float time_mult = 1.f;

	void integrate(const float dt, const plain_animations_pool& anims);

	template <bool use_neon_maps, class M>
	void draw_as_sprite(
		augs::vertex_triangle& t1,
		augs::vertex_triangle& t2,
		const M& manager,
		const plain_animations_pool& anims
	) const {
		const auto image_id = animation.get_image_id(anims);

		if constexpr(use_neon_maps) {
			augs::detail_write_neon_sprite(t1, t2, manager.at(image_id), pos, 0, color);
		}
		else {
			augs::detail_write_sprite(t1, t2, manager.at(image_id), pos, 0, color);
		}
	}

	bool is_dead() const {
		return animation.is_dead();
	}

	void set_position(const vec2);
	void set_velocity(const vec2);
	void set_acceleration(const vec2);
	void multiply_size(const float);
	void set_rotation(const float);
	void set_rotation_speed(const float);
	void set_max_lifetime_ms(const float);
	void colorize(const rgba);
};

struct homing_animated_particle {
	static constexpr std::size_t statically_allocate = 5000;

	// GEN INTROSPECTOR struct homing_animated_particle
	vec2 pos;
	vec2 vel;
	vec2 acc;

	float linear_damping = 0.f;
	float homing_force = 3000.f;

	animation_in_particle animation;
	rgba color = white;

	simple_animation_state animation_state;
	// END GEN INTROSPECTOR

	/*
		Runtime only. Bullet trail particles slow down together with the logic speed.
	*/

	float time_mult = 1.f;

	void integrate(
		const float dt, 
		const plain_animations_pool& anims,
		const vec2 homing_target
	);

	template <bool use_neon_maps, class M>
	void draw_as_sprite(
		augs::vertex_triangle& t1,
		augs::vertex_triangle& t2,
		const M& manager,
		const plain_animations_pool& anims
	) const {
		const auto image_id = animation.get_image_id(anims);

		if constexpr(use_neon_maps) {
			augs::detail_write_neon_sprite(t1, t2, manager.at(image_id), pos, 0, color);
		}
		else {
			augs::detail_write_sprite(t1, t2, manager.at(image_id), pos, 0, color);
		}
	}

	bool is_dead() const {
		return animation.is_dead();
	}

	void set_position(const vec2);
	void set_velocity(const vec2);
	void set_acceleration(const vec2);
	void multiply_size(const float);
	void set_rotation(const float);
	void set_rotation_speed(const float);
	void set_max_lifetime_ms(const float);
	void colorize(const rgba);
};

template <class T>
T& apply_to_particle(const particle_effect_modifier& m, T& p) {
	p.colorize(m.color);

	if constexpr(has_lifetime_v<T>) {
		p.max_lifetime_ms *= m.scale_lifetimes;
	}

	p.vel *= m.scale_velocities;
	p.multiply_size(m.scale_sizes);

	return p;
}