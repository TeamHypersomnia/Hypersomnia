#pragma once
#include "augs/math/vec2.h"
#include "game/components/sprite_component.h"
#include "game/detail/particle_types_declaration.h"
#include "augs/graphics/pixel.h"

struct general_particle {
	vec2 pos;
	vec2 vel;
	vec2 acc;
	components::sprite face;
	float rotation = 0.f;
	float rotation_speed = 0.f;
	float linear_damping = 0.f;
	float angular_damping = 0.f;
	float lifetime_ms = 0.f;
	float max_lifetime_ms = 0.f;
	float shrink_when_ms_remaining = 0.f;
	float unshrinking_time_ms = 0.f;

	int alpha_levels = -1;

	void integrate(const float dt);
	void draw(components::sprite::drawing_input basic_input) const;
	bool is_dead() const;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(pos),
			CEREAL_NVP(vel),
			CEREAL_NVP(acc),
			CEREAL_NVP(face),
			CEREAL_NVP(rotation),
			CEREAL_NVP(rotation_speed),
			CEREAL_NVP(linear_damping),
			CEREAL_NVP(angular_damping),
			CEREAL_NVP(lifetime_ms),
			CEREAL_NVP(max_lifetime_ms),
			CEREAL_NVP(should_disappear),
			CEREAL_NVP(ignore_rotation),
			CEREAL_NVP(alpha_levels)
		);
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

struct animated_particle {
	vec2 pos;
	vec2 vel;
	vec2 acc;
	
	float linear_damping = 0.f;
	float lifetime_ms = 0.f;

	assets::texture_id first_face = assets::texture_id::INVALID;
	rgba color;
	float frame_duration_ms;
	unsigned frame_count = 0;

	void integrate(const float dt);
	void draw(components::sprite::drawing_input basic_input) const;
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