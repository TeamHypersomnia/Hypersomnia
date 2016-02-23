#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game_framework/components/position_copying_component.h"
#include "game_framework/components/camera_component.h"
#include "game_framework/components/input_receiver_component.h"
#include "game_framework/components/crosshair_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/movement_component.h"
#include "game_framework/components/rotation_copying_component.h"
#include "game_framework/components/animation_component.h"
#include "game_framework/components/animation_response_component.h"
#include "game_framework/components/physics_component.h"
#include "game_framework/components/car_component.h"
#include "game_framework/components/trigger_component.h"
#include "game_framework/components/physics_definition_component.h"

#include "game_framework/globals/filters.h"

namespace prefabs {
	augs::entity_id create_car(augs::world& world, vec2 pos) {
		auto front = world.create_entity("front");
		auto interior = world.create_entity("interior");
		auto left_wheel = world.create_entity("left_wheel");

		front->add_sub_entity(interior);
		front->add_sub_entity(left_wheel);

		{
			auto& sprite = *front += components::sprite();
			auto& render = *front += components::render();
			auto& transform = *front += components::transform();
			auto& car = *front += components::car();
			auto& physics_definition = *front += components::physics_definition();

			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(2500, 4500)/=3;
			car.acceleration_length = 4500/5;
			transform.pos = pos;

			sprite.set(assets::texture_id::TRUCK_FRONT);
			//sprite.set(assets::texture_id::TRUCK_FRONT, augs::rgba(0, 255, 255));
			//sprite.size.x = 200;
			//sprite.size.y = 100;

			render.layer = render_layer::DYNAMIC_BODY;

			physics_definition.body.linear_damping = 0.4f;
			physics_definition.body.angular_damping = 2.f;

			auto& fixture = physics_definition.new_fixture();
			fixture.from_renderable(front);

			fixture.filter = filters::dynamic_object();
			fixture.density = 0.6f;
			
			//physics.air_resistance = 0.2f;
		}

		{
			auto& sprite = *interior += components::sprite();
			auto& render = *interior += components::render();
			auto& transform = *interior += components::transform();
			auto& physics_definition = *interior += components::physics_definition();

			transform.pos = pos;

			render.layer = render_layer::CAR_INTERIOR;

			sprite.set(assets::texture_id::TRUCK_INSIDE);
			//sprite.set(assets::texture_id::TRUCK_INSIDE, augs::rgba(122, 0, 122, 255));
			//sprite.size.x = 250;
			//sprite.size.y = 550;

			auto& fixture = physics_definition.new_fixture(front);
			fixture.from_renderable(interior);
			fixture.density = 0.6f;
			fixture.filter = filters::dynamic_object();
			
			vec2 offset(0, front->get<components::sprite>().size.y / 2 + sprite.size.y / 2);
			fixture.transform_vertices.pos = offset;
			fixture.is_friction_ground = true;
		}

		{
			auto& sprite = *left_wheel += components::sprite();
			auto& render = *left_wheel += components::render();
			auto& transform = *left_wheel += components::transform();
			auto& trigger = *left_wheel += components::trigger();
			auto& physics_definition = *left_wheel += components::physics_definition();

			transform.pos = pos;
			trigger.entity_to_be_notified = front;

			render.layer = render_layer::CAR_WHEEL;

			sprite.set(assets::texture_id::CAR_INSIDE, augs::rgba(29, 0, 0, 255));
			sprite.size.x = 30;
			sprite.size.y = 60;

			auto& fixture = physics_definition.new_fixture(front);

			fixture.from_renderable(left_wheel);
			fixture.density = 0.6f;
			fixture.filter = filters::trigger();
			fixture.sensor = true;

			vec2 offset(0, front->get<components::sprite>().size.y / 2 + sprite.size.y / 2 + 20);
			fixture.transform_vertices.pos = offset;
		}

		return front;
	}

}