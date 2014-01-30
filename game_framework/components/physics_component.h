#pragma once
#include "entity_system/component.h"
#define METERS_TO_PIXELS 50.0
#define PIXELS_TO_METERS 1.0/METERS_TO_PIXELS
#define METERS_TO_PIXELSf 50.f
#define PIXELS_TO_METERSf 1.0f/METERS_TO_PIXELSf

namespace components {
	struct physics : public augs::entity_system::component {
		b2Body* body;
		std::vector <augs::vec2<>> original_model;

		bool enable_angle_motor;
		float target_angle;

		physics(b2Body* body = nullptr) : body(body), enable_angle_motor(false), target_angle(0.f)	{}
	};
}
