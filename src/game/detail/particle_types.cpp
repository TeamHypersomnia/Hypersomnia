#include "particle_types.h"

#include "game/assets/assets_manager.h"

template <class T>
inline void integrate_pos_vel_acc_damp_life(T& p, const float dt) {
	p.vel += p.acc * dt;
	p.pos += p.vel * dt;
	
	p.vel.damp(p.linear_damping * dt);

	p.current_lifetime_ms += dt * 1000.f;
}

void general_particle::integrate(const float dt) {
	integrate_pos_vel_acc_damp_life(*this, dt);
	rotation += rotation_speed * dt;

	augs::damp(rotation_speed, angular_damping * dt);
}

bool general_particle::is_dead() const {
	return current_lifetime_ms >= max_lifetime_ms;
}

void general_particle::draw(components::sprite::drawing_input basic_input) const {
	float size_mult = 1.f;

	if (shrink_when_ms_remaining > 0.f) {
		const auto alivity_multiplier = std::min(1.f, (max_lifetime_ms - current_lifetime_ms) / shrink_when_ms_remaining);

		size_mult *= sqrt(alivity_multiplier);

		//const auto desired_alpha = static_cast<rgba_channel>(alivity_multiplier * static_cast<float>(temp_alpha));
		//
		//if (fade_on_disappearance) {
		//	if (alpha_levels > 0) {
		//		face.color.a = desired_alpha == 0 ? 0 : ((255 / alpha_levels) * (1 + (desired_alpha / (255 / alpha_levels))));
		//	}
		//	else {
		//		face.color.a = desired_alpha;
		//	}
		//}
	}

	if (unshrinking_time_ms > 0.f) {
		size_mult *= std::min(1.f, (current_lifetime_ms / unshrinking_time_ms)*(current_lifetime_ms / unshrinking_time_ms));
	}

	components::sprite f;
	f.set(image_id, size_mult * size, color);

	basic_input.renderable_transform = { pos, rotation };
	f.draw(basic_input);
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
	const assets::game_image_id id,
	const rgba col
) {
	set_image(id, get_assets_manager()[id].get_size(), col);
}

void general_particle::set_image(
	const assets::game_image_id id,
	vec2 s,
	const rgba col
) {
	image_id = id;
	size = s;
	color = col;
}

void animated_particle::integrate(const float dt) {
	integrate_pos_vel_acc_damp_life(*this, dt);
}

void animated_particle::draw(components::sprite::drawing_input basic_input) const {
	thread_local components::sprite face;
	const auto frame_num = std::min(static_cast<unsigned>(current_lifetime_ms / frame_duration_ms), frame_count - 1);

	const auto target_id = static_cast<assets::game_image_id>(static_cast<int>(first_face) + frame_num);

	face.set(
		target_id,
		get_assets_manager()[target_id].get_size()
	);
	
	basic_input.renderable_transform = { pos, 0 };
	face.color = color;
	face.draw(basic_input);
}

bool animated_particle::is_dead() const {
	return current_lifetime_ms >= frame_duration_ms * frame_count;
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
	const vec2 homing_target
) {
	vel += (homing_target - pos) * 10 * dt;
	
	vec2 dirs[] = { vel.perpendicular_cw(), -vel.perpendicular_cw() };

	const auto homing_vector = homing_target - pos;

	if (dirs[1].dot(homing_vector) > dirs[0].dot(homing_vector)) {
		std::swap(dirs[0], dirs[1]);
	}

	vel += dirs[0].set_length(sqrt(sqrt(homing_vector.length()))) * homing_force * dt;

	integrate_pos_vel_acc_damp_life(*this, dt);
}

void homing_animated_particle::draw(components::sprite::drawing_input basic_input) const {
	thread_local components::sprite face;
	const auto frame_num = std::min(static_cast<unsigned>(current_lifetime_ms / frame_duration_ms), frame_count-1);

	//face.set(static_cast<assets::game_image_id>(static_cast<int>(first_face) + frame_count - frame_num - 1));
	const auto target_id = static_cast<assets::game_image_id>(static_cast<int>(first_face) + frame_num);

	face.set(
		target_id,
		get_assets_manager()[target_id].get_size()
	);

	basic_input.renderable_transform = { pos, 0 };
	face.color = color;
	face.draw(basic_input);
}

bool homing_animated_particle::is_dead() const {
	return current_lifetime_ms >= frame_duration_ms * frame_count;
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
