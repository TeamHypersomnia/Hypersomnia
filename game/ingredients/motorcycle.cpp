#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game/components/position_copying_component.h"
#include "game/components/camera_component.h"
#include "game/components/input_receiver_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/animation_component.h"
#include "game/components/animation_response_component.h"
#include "game/components/physics_definition_component.h"
#include "game/components/car_component.h"
#include "game/components/trigger_component.h"
#include "game/components/name_component.h"

#include "game/globals/filters.h"

namespace prefabs {
	augs::entity_id create_motorcycle(augs::world& world, components::transform spawn_transform) {
		auto front = world.create_entity("front");
		auto interior = world.create_entity("interior");
		auto left_wheel = world.create_entity("left_wheel");

		front->add_sub_entity(interior);
		front->add_sub_entity(left_wheel);
		name_entity(front, entity_name::MOTORCYCLE);

		{
			auto& sprite = *front += components::sprite();
			auto& render = *front += components::render();
			auto& transform = *front += spawn_transform;
			auto& car = *front += components::car();
			auto& physics_definition = *front += components::physics_definition();

			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(1000, 1000);
			car.acceleration_length = 3500;
			
			car.wheel_offset = vec2(35, 0);
			car.maximum_lateral_cancellation_impulse = 50;
			car.static_air_resistance = 0.0006;
			car.dynamic_air_resistance = 0.00009;
			car.static_damping = 2;
			car.dynamic_damping = 0.2;
			car.angular_damping_while_hand_braking = 0.0;
			car.angular_damping = 0.0;
			car.maximum_speed_with_static_air_resistance = 1000;
			car.maximum_speed_with_static_damping = 0;
			car.braking_damping = 2.f;
			
			car.minimum_speed_for_maneuverability_decrease = -1;
			car.maneuverability_decrease_multiplier = 0.00006f;

			car.lateral_impulse_multiplier = 0.3;
			car.braking_angular_damping = 16.f;

			sprite.set(assets::texture_id::MOTORCYCLE_FRONT);
			render.layer = render_layer::CAR_WHEEL;

			auto& body = physics_definition.body;
			body.linear_damping = 0.4f;
			body.angular_damping = 2.f;
			body.angular_air_resistance = 0.55f;

			auto& info = physics_definition.new_fixture();
			info.from_renderable(front);

			info.filter = filters::dynamic_object();
			info.density = 0.6f;
			info.restitution = 0.3;

			car.angular_air_resistance = 0.55f;
			car.angular_air_resistance_while_hand_braking = 0.05f;
		}

		{
			auto& sprite = *interior += components::sprite();
			auto& render = *interior += components::render();
			auto& transform = *interior += spawn_transform;
			auto& physics_definition = *interior += components::physics_definition();

			render.layer = render_layer::CAR_WHEEL;

			sprite.set(assets::texture_id::MOTORCYCLE_INSIDE);

			auto& info = physics_definition.new_fixture(front);
			info.from_renderable(interior);
			info.density = 0.6f;
			info.sensor = true;
			info.filter = filters::dynamic_object();
			vec2 offset((front->get<components::sprite>().size.x / 2 + sprite.size.x / 2 - 1) * -1, 0);
			info.transform_vertices.pos = offset;
		}

		{
			auto& sprite = *left_wheel += components::sprite();
			auto& render = *left_wheel += components::render();
			auto& transform = *left_wheel += spawn_transform;
			auto& trigger = *left_wheel += components::trigger();
			auto& physics_definition = *left_wheel += components::physics_definition();

			trigger.entity_to_be_notified = front;
			trigger.react_to_collision_detectors = false;
			trigger.react_to_query_detectors = true;

			render.layer = render_layer::CAR_WHEEL;

			sprite.set(assets::texture_id::CAR_INSIDE, augs::rgba(255, 0, 0, 255));
			sprite.size.set(20, 10);
			sprite.color.a = 0;

			auto& info = physics_definition.new_fixture(front);
			info.from_renderable(left_wheel);
			info.density = 0.6f;
			info.filter = filters::trigger();
			info.sensor = true;
			vec2 offset((front->get<components::sprite>().size.x / 2 + sprite.size.x / 2) * -1, 0);
			info.transform_vertices.pos = offset;
		}

		return front;
	}

}