#pragma once
#include "entity_system/component.h"
#include "math/vec2.h"
extern double METERS_TO_PIXELS;
extern double PIXELS_TO_METERS;
extern float METERS_TO_PIXELSf;
extern float PIXELS_TO_METERSf;

class b2Body;
namespace components {
	struct physics : public augs::component {
		b2Body* body = nullptr;
		std::vector <vec2> original_model;

		bool enable_angle_motor = false;
		float target_angle = 0.f;
		float angle_motor_force_multiplier = 1.f;

		/* a physically realistic alternative to max_speed variable, the bigger the value is, the lesser the maximum speed */
		float air_resistance = 0.f;

		vec2 velocity();
		void set_velocity(vec2);
		void set_linear_damping_vec(vec2);
		void apply_force(vec2);
		float get_mass();
	};
}
