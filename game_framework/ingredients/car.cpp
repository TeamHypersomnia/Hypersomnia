#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game_framework/components/chase_component.h"
#include "game_framework/components/camera_component.h"
#include "game_framework/components/input_component.h"
#include "game_framework/components/crosshair_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/movement_component.h"
#include "game_framework/components/lookat_component.h"
#include "game_framework/components/animation_component.h"
#include "game_framework/components/animation_response_component.h"
#include "game_framework/components/physics_component.h"
#include "game_framework/components/children_component.h"
#include "game_framework/components/car_component.h"
#include "game_framework/components/trigger_component.h"

#include "game_framework/shared/physics_setup_helpers.h"

#include "game_framework/globals/filters.h"

namespace prefabs {
	augs::entity_id create_car(augs::world& world, vec2 pos) {
		auto front = world.create_entity("front");
		auto interior = world.create_entity("interior");
		auto left_wheel = world.create_entity("left_wheel");

		auto& car_children = *front += components::children();
		car_children.add_sub_entity(interior);
		car_children.add_sub_entity(left_wheel);

		{
			auto& sprite = *front += components::sprite();
			auto& render = *front += components::render();
			auto& transform = *front += components::transform();
			auto& car = *front += components::car();

			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(2500, 4500)/=3;
			car.acceleration_length = 4500/5;
			transform.pos = pos;

			sprite.set(assets::texture_id::TRUCK_FRONT);
			//sprite.set(assets::texture_id::TRUCK_FRONT, augs::rgba(0, 255, 255));
			//sprite.size.x = 200;
			//sprite.size.y = 100;

			render.layer = render_layer::DYNAMIC_BODY;

			body_definition body;
			body.linear_damping = 0.4f;
			body.angular_damping = 2.f;

			fixture_definition info;
			info.from_renderable(front);

			info.filter = filters::dynamic_object();
			info.density = 0.6f;

			auto& physics = create_physics_component(body, front);
			add_fixtures(info, front);

			//physics.air_resistance = 0.2f;
		}

		{
			auto& sprite = *interior += components::sprite();
			auto& render = *interior += components::render();
			auto& transform = *interior += components::transform();

			transform.pos = pos;

			render.layer = render_layer::CAR_INTERIOR;

			sprite.set(assets::texture_id::TRUCK_INSIDE);
			//sprite.set(assets::texture_id::TRUCK_INSIDE, augs::rgba(122, 0, 122, 255));
			//sprite.size.x = 250;
			//sprite.size.y = 550;

			fixture_definition info;
			info.from_renderable(interior);
			info.density = 0.6f;
			info.filter = filters::dynamic_object();
			vec2 offset(0, front->get<components::sprite>().size.y / 2 + sprite.size.y / 2);
			info.transform_vertices.pos = offset;

			auto& fixtures = add_fixtures_to_other_body(info, interior, front);
			fixtures.is_friction_ground = true;
		}

		{
			auto& sprite = *left_wheel += components::sprite();
			auto& render = *left_wheel += components::render();
			auto& transform = *left_wheel += components::transform();
			auto& trigger = *left_wheel += components::trigger();

			transform.pos = pos;
			trigger.entity_to_be_notified = front;

			render.layer = render_layer::CAR_WHEEL;

			sprite.set(assets::texture_id::CAR_INSIDE, augs::rgba(29, 0, 0, 255));
			sprite.size.x = 30;
			sprite.size.y = 60;

			fixture_definition info;
			info.from_renderable(left_wheel);
			info.density = 0.6f;
			info.filter = filters::trigger();
			info.sensor = true;
			vec2 offset(0, front->get<components::sprite>().size.y / 2 + sprite.size.y / 2 + 20);
			info.transform_vertices.pos = offset;
			
			auto& physics = add_fixtures_to_other_body(info, left_wheel, front);
		}

		return front;
	}

}