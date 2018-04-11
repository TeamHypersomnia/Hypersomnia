#pragma once
#include "augs/math/vec2.h"
#include "game/components/sprite_component.h"
#include "view/viewables/particle_types_declaration.h"
#include "augs/graphics/rgba.h"
#include "game/transcendental/entity_id.h"

struct general_particle {
	// GEN INTROSPECTOR struct general_particle
	vec2 pos;
	vec2 vel;
	vec2 acc;
	assets::image_id image_id;
	rgba color;
	vec2 size;
	float rotation = 0.f;
	float rotation_speed = 0.f;
	float linear_damping = 0.f;
	float angular_damping = 0.f;
	float current_lifetime_ms = 0.f;
	float max_lifetime_ms = 0.f;
	float shrink_when_ms_remaining = 0.f;
	float unshrinking_time_ms = 0.f;

	int alpha_levels = -1;
	// END GEN INTROSPECTOR

	void integrate(const float dt);

	template <class M>
	void draw_as_sprite(
		const M& manager,
		invariants::sprite::drawing_input basic_input
	) const {
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

		invariants::sprite f;
		f.set(image_id, size_mult * size, color);

		basic_input.renderable_transform = { pos, rotation };
		f.draw(manager, basic_input);
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

	void set_image(
		const assets::image_id,
		vec2 size,
		const rgba
	);
};

struct animated_particle {
	// GEN INTROSPECTOR struct animated_particle
	vec2 pos;
	vec2 vel;
	vec2 acc;
	
	float linear_damping = 0.f;
	float current_lifetime_ms = 0.f;

	int starting_frame_num = 0;
	rgba color;
	float frame_duration_ms = 0.f;
	unsigned frame_count = 0;
	// END GEN INTROSPECTOR

	void integrate(const float dt);

	template <class M>
	void draw_as_sprite(
		const M& manager,
		invariants::sprite::drawing_input basic_input
	) const {
		thread_local invariants::sprite face;
		const auto frame_num = std::min(static_cast<unsigned>(current_lifetime_ms / frame_duration_ms), frame_count - 1);

		const auto target_id = starting_frame_num + frame_num;

		face.set(target_id, manager.at(target_id).get_size(), white);

		basic_input.renderable_transform = { pos, 0 };
		face.color = color;
		face.draw(manager, basic_input);
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
};

struct homing_animated_particle {
	// GEN INTROSPECTOR struct homing_animated_particle
	vec2 pos;
	vec2 vel;
	vec2 acc;

	float linear_damping = 0.f;
	float current_lifetime_ms = 0.f;

	float homing_force = 3000.f;

	assets::image_id starting_frame_num;
	rgba color;
	float frame_duration_ms = 0.f;
	unsigned frame_count = 0;
	// END GEN INTROSPECTOR

	void integrate(
		const float dt, 
		const vec2 homing_target
	);

	template <class M>
	void draw_as_sprite(
		const M& manager,
		invariants::sprite::drawing_input basic_input
	) const {
		thread_local invariants::sprite face;
		const auto frame_num = std::min(static_cast<unsigned>(current_lifetime_ms / frame_duration_ms), frame_count - 1);

		const auto target_id = starting_frame_num + frame_num;

		face.set(
			target_id,
			manager.at(target_id).get_size()
		);

		basic_input.renderable_transform = { pos, 0 };
		face.color = color;
		face.draw(manager, basic_input);
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
};