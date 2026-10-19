#pragma once
#include <cmath>
#include <algorithm>
#include "particle_types.h"
#include "augs/templates/traits/has_rotation.h"

template <class T>
FORCE_INLINE void generic_integrate_particle(T& p, const float dt) {
	p.vel += p.acc * dt;
	p.pos += p.vel * dt;
	
	p.vel.shrink(p.linear_damping * dt);

	if constexpr(has_lifetime_v<T>) {
		p.current_lifetime_ms += dt * 1000;
	}

	if constexpr(has_rotation_v<T>) {
		p.rotation += p.rotation_speed * dt;
		augs::shrink(p.rotation_speed, p.angular_damping * dt);
	}
}

FORCE_INLINE void general_particle::integrate(const float dt) {
	/*
		Slows down by the same fraction every second, unlike linear_damping's constant deceleration.
	*/

	if (exponential_damping > 0.f) {
		vel *= std::exp(-exponential_damping * dt);
	}

	generic_integrate_particle(*this, dt);
}

FORCE_INLINE bool general_particle::is_dead() const {
	return current_lifetime_ms >= max_lifetime_ms;
}

FORCE_INLINE void explosion_particle::update_sprite_transform() {
	sprite.pos = center + vec2::from_degrees(angle) * radius + drift;
	sprite.rotation = angle + 90.f + rotation_offset;
}

FORCE_INLINE float explosion_particle::calc_lifetime_alpha() const {
	const auto age_ms = sprite.current_lifetime_ms;
	auto result = max_alpha;

	if (fade_in_ms > 0.f) {
		result *= std::min(1.f, age_ms / fade_in_ms);
	}

	if (fade_out_ms > 0.f) {
		result *= std::clamp((sprite.max_lifetime_ms - age_ms) / fade_out_ms, 0.f, 1.f);
	}

	return result;
}

FORCE_INLINE void explosion_particle::begin_edge_fade() {
	edge_fade_from_alpha = calc_lifetime_alpha();
	edge_fade_start_ms = sprite.current_lifetime_ms;
	lifetime_before_edge_fade_ms = sprite.max_lifetime_ms;
	sprite.max_lifetime_ms = sprite.current_lifetime_ms + edge_fade_ms;
}

FORCE_INLINE void explosion_particle::update_sprite_color() {
	const auto age_ms = sprite.current_lifetime_ms;

	const auto alpha = [&]() {
		const auto result = 
			is_edge_fading() ?
			edge_fade_from_alpha * std::clamp(1.f - (age_ms - edge_fade_start_ms) / edge_fade_ms, 0.f, 1.f) :
			calc_lifetime_alpha()
		;

		return static_cast<rgba_channel>(255.f * result);
	}();

	if (!palette.empty()) {
		sprite.color = ::sample_explosion_particles_palette(palette, palette_offset + age_ms / palette_step_ms);
		sprite.color.a = alpha;
		return;
	}

	const auto considered = [&]() {
		if (age_ms < hot_until_ms) {
			return augs::interp(hot_color, ring_color, age_ms / hot_until_ms);
		}

		if (age_ms > cool_from_ms) {
			const auto cooling_ms = std::max(cool_until_ms - cool_from_ms, 1.f);
			return augs::interp(ring_color, cool_color, std::min(1.f, (age_ms - cool_from_ms) / cooling_ms));
		}

		return ring_color;
	}();

	sprite.color = considered;
	sprite.color.a = alpha;
}

FORCE_INLINE void explosion_particle::integrate(const float dt) {
	sprite.current_lifetime_ms += dt * 1000;

	if (velocity_damping > 0.f) {
		const auto decay = std::exp(-velocity_damping * dt);

		radial_vel *= decay;
		tangential_vel *= decay;
	}

	radius += radial_vel * dt;
	angle += RAD_TO_DEG<float> * tangential_vel * dt / std::max(radius, 1.f) + angular_vel * dt;

	drift_vel += drift_acc * dt;
	drift += drift_vel * dt;

	if (radius > max_radius && stop_at_max_radius) {
		radius = max_radius;
		radial_vel = 0.f;
	}

	if (radius < 0.f || radius > max_radius) {
		sprite.current_lifetime_ms = sprite.max_lifetime_ms;
	}

	if (edge_fade_ms > 0.f && !is_edge_fading() && radius >= edge_fade_radius && !is_dead()) {
		begin_edge_fade();
	}

	update_sprite_transform();
	update_sprite_color();
}

FORCE_INLINE void general_particle::set_position(const vec2 new_pos) {
	pos = new_pos;
}

FORCE_INLINE void general_particle::set_velocity(const vec2 new_vel) {
	vel = new_vel;
}

FORCE_INLINE void general_particle::set_acceleration(const vec2 new_acc) {
	acc = new_acc;
}

FORCE_INLINE void general_particle::multiply_size(const float mult) {
	size = vec2(size) * mult;
}

FORCE_INLINE void general_particle::set_rotation(const float new_rotation) {
	rotation = new_rotation;
}

FORCE_INLINE void general_particle::set_rotation_speed(const float new_rotation_speed) {
	rotation_speed = new_rotation_speed;
}

FORCE_INLINE void general_particle::set_max_lifetime_ms(const float new_max_lifetime_ms) {
	max_lifetime_ms = new_max_lifetime_ms;
}

FORCE_INLINE void general_particle::colorize(const rgba mult) {
	color *= mult;
	start_color *= mult;
}

FORCE_INLINE void general_particle::set_image(
	const assets::image_id id,
	const vec2i s,
	const rgba col
) {
	image_id = id;
	size = s;
	color = col;
}

FORCE_INLINE void animated_particle::integrate(const float dt, const plain_animations_pool& anims) {
	if (animation.should_integrate(anims)) {
		generic_integrate_particle(*this, dt);
	}

	animation.advance(dt * 1000, anims);
}

FORCE_INLINE void animated_particle::set_position(const vec2 new_pos) {
	pos = new_pos;
}

FORCE_INLINE void animated_particle::set_velocity(const vec2 new_vel) {
	vel = new_vel;
}

FORCE_INLINE void animated_particle::set_acceleration(const vec2 new_acc) {
	acc = new_acc;
}

FORCE_INLINE void animated_particle::multiply_size(const float /* mult */) {

}

FORCE_INLINE void animated_particle::set_rotation(const float /* new_rotation */) {

}

FORCE_INLINE void animated_particle::set_rotation_speed(const float /* new_rotation_speed */) {

}

FORCE_INLINE void animated_particle::set_max_lifetime_ms(const float /* new_max_lifetime_ms */) {

}

FORCE_INLINE void animated_particle::colorize(const rgba mult) {
	color *= mult;
}

FORCE_INLINE void homing_animated_particle::integrate(
	const float dt, 
	const plain_animations_pool& anims,
	const vec2 homing_target
) {
	vel += (homing_target - pos) * 10 * dt;
	
	vec2 dirs[] = { vel.perpendicular_cw(), -vel.perpendicular_cw() };

	const auto homing_vector = homing_target - pos;

	if (dirs[1].dot(homing_vector) > dirs[0].dot(homing_vector)) {
		std::swap(dirs[0], dirs[1]);
	}

	vel += dirs[0].set_length(std::sqrt(std::sqrt(homing_vector.length()))) * homing_force * dt;

	generic_integrate_particle(*this, dt);
	animation.advance(dt * 1000, anims);
}

FORCE_INLINE void homing_animated_particle::set_position(const vec2 new_pos) {
	pos = new_pos;
}

FORCE_INLINE void homing_animated_particle::set_velocity(const vec2 new_vel) {
	vel = new_vel;
}

FORCE_INLINE void homing_animated_particle::set_acceleration(const vec2 new_acc) {
	acc = new_acc;
}

FORCE_INLINE void homing_animated_particle::multiply_size(const float /* mult */) {

}

FORCE_INLINE void homing_animated_particle::set_rotation(const float /* new_rotation */) {

}

FORCE_INLINE void homing_animated_particle::set_rotation_speed(const float /* new_rotation_speed */) {

}

FORCE_INLINE void homing_animated_particle::set_max_lifetime_ms(const float /* new_max_lifetime_ms */) {

}

FORCE_INLINE void homing_animated_particle::colorize(const rgba mult) {
	color *= mult;
}
