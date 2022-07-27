#pragma once
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
	generic_integrate_particle(*this, dt);
}

FORCE_INLINE bool general_particle::is_dead() const {
	return current_lifetime_ms >= max_lifetime_ms;
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
