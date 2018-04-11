#include "particle_types.h"

template <class T, class = void>
struct has_rotation : std::false_type {};

template <class T>
struct has_rotation<T, decltype(std::declval<T&>().rotation, void())> : std::true_type {};

template <class T, class = void>
struct has_lifetime : std::false_type {};

template <class T>
struct has_lifetime<T, decltype(std::declval<T&>().current_lifetime_ms, void())> : std::true_type {};

template <class T>
inline void generic_integrate_particle(T& p, const float dt) {
	p.vel += p.acc * dt;
	p.pos += p.vel * dt;
	
	p.vel.shrink(p.linear_damping * dt);

	if constexpr(has_lifetime<T>::value) {
		p.current_lifetime_ms += dt * 1000;
	}

	if constexpr(has_rotation<T>::value) {
		p.rotation += p.rotation_speed * dt;
		augs::shrink(p.rotation_speed, p.angular_damping * dt);
	}
}

void general_particle::integrate(const float dt) {
	generic_integrate_particle(*this, dt);
}

bool general_particle::is_dead() const {
	return current_lifetime_ms >= max_lifetime_ms;
}

void general_particle::set_position(const vec2 new_pos) {
	pos = new_pos;
}

void general_particle::set_velocity(const vec2 new_vel) {
	vel = new_vel;
}

void general_particle::set_acceleration(const vec2 new_acc) {
	acc = new_acc;
}

void general_particle::multiply_size(const float mult) {
	size *= mult;
}

void general_particle::set_rotation(const float new_rotation) {
	rotation = new_rotation;
}

void general_particle::set_rotation_speed(const float new_rotation_speed) {
	rotation_speed = new_rotation_speed;
}

void general_particle::set_max_lifetime_ms(const float new_max_lifetime_ms) {
	max_lifetime_ms = new_max_lifetime_ms;
}

void general_particle::colorize(const rgba mult) {
	color *= mult;
}

void general_particle::set_image(
	const assets::image_id id,
	vec2 s,
	const rgba col
) {
	image_id = id;
	size = s;
	color = col;
}

void animated_particle::integrate(const float dt, const animations_pool& anims) {
	generic_integrate_particle(*this, dt);

	animation.advance(dt * 1000, anims);
}

void animated_particle::set_position(const vec2 new_pos) {
	pos = new_pos;
}

void animated_particle::set_velocity(const vec2 new_vel) {
	vel = new_vel;
}

void animated_particle::set_acceleration(const vec2 new_acc) {
	acc = new_acc;
}

void animated_particle::multiply_size(const float mult) {

}

void animated_particle::set_rotation(const float new_rotation) {

}

void animated_particle::set_rotation_speed(const float new_rotation_speed) {

}

void animated_particle::set_max_lifetime_ms(const float new_max_lifetime_ms) {

}

void animated_particle::colorize(const rgba mult) {
	color *= mult;
}

void homing_animated_particle::integrate(
	const float dt, 
	const vec2 homing_target,
	const animations_pool& anims
) {
	vel += (homing_target - pos) * 10 * dt;
	
	vec2 dirs[] = { vel.perpendicular_cw(), -vel.perpendicular_cw() };

	const auto homing_vector = homing_target - pos;

	if (dirs[1].dot(homing_vector) > dirs[0].dot(homing_vector)) {
		std::swap(dirs[0], dirs[1]);
	}

	vel += dirs[0].set_length(sqrt(sqrt(homing_vector.length()))) * homing_force * dt;

	generic_integrate_particle(*this, dt);
	animation.advance(dt * 1000, anims);
}

void homing_animated_particle::set_position(const vec2 new_pos) {
	pos = new_pos;
}

void homing_animated_particle::set_velocity(const vec2 new_vel) {
	vel = new_vel;
}

void homing_animated_particle::set_acceleration(const vec2 new_acc) {
	acc = new_acc;
}

void homing_animated_particle::multiply_size(const float mult) {

}

void homing_animated_particle::set_rotation(const float new_rotation) {

}

void homing_animated_particle::set_rotation_speed(const float new_rotation_speed) {

}

void homing_animated_particle::set_max_lifetime_ms(const float new_max_lifetime_ms) {

}

void homing_animated_particle::colorize(const rgba mult) {
	color *= mult;
}
