#include "ingredients.h"

#include "game/systems_stateless/particles_existence_system.h"
#include "game/systems_stateless/sound_existence_system.h"
#include "game/components/position_copying_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/animation_component.h"
#include "game/components/animation_response_component.h"
#include "game/components/physics_component.h"
#include "game/components/car_component.h"
#include "game/components/trigger_component.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/name_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/sound_existence_component.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"

namespace prefabs {
	entity_handle create_car(cosmos& world, const components::transform& spawn_transform) {
		auto front = world.create_entity("front");
		auto interior = world.create_entity("interior");
		auto left_wheel = world.create_entity("left_wheel");


		front.add_sub_entity(interior);
		front.add_sub_entity(left_wheel);
		name_entity(front, entity_name::TRUCK);

		{
			auto& sprite = front += components::sprite();
			auto& render = front += components::render();
			auto& car = front += components::car();
			components::physics physics_definition(spawn_transform);
			components::fixtures colliders;

			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(2500, 4500) /= 3;
			car.acceleration_length = 4500 / 5;

			sprite.set(assets::texture_id::TRUCK_FRONT);
			//sprite.set(assets::texture_id::TRUCK_FRONT, augs::rgba(0, 255, 255));
			//sprite.size.x = 200;
			//sprite.size.y = 100;

			render.layer = render_layer::DYNAMIC_BODY;

			physics_definition.linear_damping = 0.4f;
			physics_definition.angular_damping = 2.f;

			auto& fixture = colliders.new_collider();
			fixture.shape.from_renderable(front);

			fixture.filter = filters::dynamic_object();
			fixture.density = 0.6f;

			front += physics_definition;
			front += colliders;
			front.get<components::fixtures>().set_owner_body(front);
			//physics.air_resistance = 0.2f;
		}

		vec2 rear_offset;
		{
			auto& sprite = interior += components::sprite();
			auto& render = interior += components::render();
			components::fixtures colliders;

			render.layer = render_layer::CAR_INTERIOR;

			sprite.set(assets::texture_id::TRUCK_INSIDE);
			//sprite.set(assets::texture_id::TRUCK_INSIDE, augs::rgba(122, 0, 122, 255));
			//sprite.size.x = 250;
			//sprite.size.y = 550;

			auto& fixture = colliders.new_collider();
			fixture.shape.from_renderable(interior);
			fixture.density = 0.6f;
			fixture.filter = filters::friction_ground();

			vec2 offset((front.get<components::sprite>().size.x / 2 + sprite.size.x / 2) * -1, 0);
			rear_offset = front.get<components::sprite>().size + sprite.size;

			colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
			colliders.is_friction_ground = true;

			interior += colliders;

			interior.get<components::fixtures>().set_owner_body(front);
		}

		{
			auto& sprite = left_wheel += components::sprite();
			auto& render = left_wheel += components::render();
			auto& trigger = left_wheel += components::trigger();
			components::fixtures colliders;

			trigger.entity_to_be_notified = front;
			trigger.react_to_collision_detectors = false;
			trigger.react_to_query_detectors = true;

			render.layer = render_layer::CAR_WHEEL;

			sprite.set(assets::texture_id::CAR_INSIDE, augs::rgba(29, 0, 0, 0));
			sprite.size.set(60, 30);

			auto& fixture = colliders.new_collider();

			fixture.shape.from_renderable(left_wheel);
			fixture.density = 0.6f;
			fixture.filter = filters::trigger();
			fixture.sensor = true;

			vec2 offset((front.get<components::sprite>().size.x / 2 + sprite.size.x / 2 + 20) * -1, 0);
			colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;

			left_wheel += colliders;

			left_wheel.get<components::fixtures>().set_owner_body(front);
		}

		{
			messages::create_particle_effect effect;
			effect.place_of_birth = spawn_transform;
			effect.place_of_birth.pos -= vec2(rear_offset.x - 80.f, 0).rotate(effect.place_of_birth.rotation, vec2());
			effect.place_of_birth.rotation += 180;
			effect.input.effect = assets::particle_effect_id::ENGINE_PARTICLES;
			effect.input.randomize_position_within_radius = 15.f;
			effect.input.single_displacement_duration_ms.set(200.f, 1000.f);
			effect.subject = front;
			effect.input.modifier.colorize = cyan;
			effect.input.modifier.scale_amounts = 1.5f;
			effect.input.modifier.scale_lifetimes = 1.2f;
			effect.input.delete_entity_after_effect_lifetime = false;

			const auto rear_engine = particles_existence_system().create_particle_effect_entity(world, effect);

			auto& existence = rear_engine.get<components::particles_existence>();
			existence.distribute_within_segment_of_length = interior.get<components::sprite>().size.y * 0.8;

			rear_engine.add_standard_components();
			front.add_sub_entity(rear_engine);
			front.get<components::car>().acceleration_engine = rear_engine;
			components::particles_existence::deactivate(rear_engine);

			{
				components::sound_existence::effect_input in;
				in.effect = assets::sound_buffer_id::ENGINE;
				in.delete_entity_after_effect_lifetime = false;
				const auto rear_engine_sound = sound_existence_system().create_sound_effect_entity(world, in, rear_engine.logic_transform(), rear_engine);
				rear_engine_sound.add_standard_components();
				rear_engine.add_sub_entity(rear_engine_sound);
				components::sound_existence::deactivate(rear_engine_sound);
			}
		}

		front.add_standard_components();
		left_wheel.add_standard_components();
		interior.add_standard_components();

		return front;
	}

}