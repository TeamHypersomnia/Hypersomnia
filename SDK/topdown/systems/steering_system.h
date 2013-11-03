#pragma once
#include "entity_system/processing_system.h"

#include "../components/steering_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

using namespace augmentations;
using namespace entity_system;

class steering_system : public processing_system_templated<components::transform, components::physics, components::steering> {
public:
	void substep(world&) override;
	void process_entities(world&) override;
	void process_events(world&) override;

	vec2<> steering_system::seek(vec2<> position, vec2<> velocity, vec2<> target, float max_speed, float arrival_radius);
	vec2<> steering_system::flee(vec2<> position, vec2<> velocity, vec2<> target, float max_speed, float flee_radius);
	vec2<> predict_interception(vec2<> position, vec2<> velocity, vec2<> target, vec2<> target_velocity, float max_prediction_ms);
};