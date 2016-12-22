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
#include "game/components/polygon_component.h"
#include "game/transcendental/cosmos.h"

#include "game/resources/manager.h"

#include "game/enums/filters.h"

namespace prefabs {
	entity_handle create_car(cosmos& world, const components::transform& spawn_transform) {
		auto front = world.create_entity("front");
		auto interior = world.create_entity("interior");
		auto left_wheel = world.create_entity("left_wheel");


		front.add_sub_entity(interior);
		front.add_sub_entity(left_wheel);
		name_entity(front, entity_name::TRUCK);

		const vec2 front_size = get_resource_manager().find(assets::texture_id::TRUCK_FRONT)->img.get_size();
		const vec2 interior_size = get_resource_manager().find(assets::texture_id::TRUCK_INSIDE)->img.get_size();

		{
			auto& sprite = front += components::polygon();
			auto& render = front += components::render();
			auto& car = front += components::car();
			components::physics physics_definition(spawn_transform);
			components::fixtures colliders;

			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(2500, 4500) /= 3;
			car.acceleration_length = 4500 / 5;
			//car.acceleration_length = 4500 / 6.0;

			sprite.from_polygonized_texture(assets::texture_id::TRUCK_FRONT);
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

			vec2 offset((front_size.x / 2 + sprite.size.x / 2) * -1, 0);

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

			vec2 offset((front_size.x / 2 + sprite.size.x / 2 + 20) * -1, 0);
			colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;

			left_wheel += colliders;

			left_wheel.get<components::fixtures>().set_owner_body(front);
		}

		{
			for (int i = 0; i < 4; ++i) {
					const auto engine_physical = world.create_entity("engine_body");

					auto& sprite = engine_physical += components::sprite();
					auto& render = engine_physical += components::render();
					components::fixtures colliders;

					render.layer = render_layer::SMALL_DYNAMIC_BODY;

					sprite.set(assets::texture_id::TRUCK_ENGINE);
					//sprite.set(assets::texture_id::TRUCK_INSIDE, augs::rgba(122, 0, 122, 255));
					//sprite.size.x = 250;
					//sprite.size.y = 550;

					auto& fixture = colliders.new_collider();
					fixture.shape.from_renderable(engine_physical);
					fixture.density = 1.0f;
					fixture.filter = filters::see_through_dynamic_object();

					components::transform offset;

					if (i == 0) {
						offset.pos.set((front_size.x / 2 + interior_size.x + sprite.size.x / 2 - 5.f) * -1, (-interior_size.y / 2 + sprite.size.y / 2));
					}
					if (i == 1) {
						offset.pos.set((front_size.x / 2 + interior_size.x + sprite.size.x / 2 - 5.f) * -1, -(-interior_size.y / 2 + sprite.size.y / 2));
					}
					if (i == 2) {
						offset.pos.set(-100, (interior_size.y / 2 + sprite.size.x / 2) * -1);
						offset.rotation = -90;
					}
					if (i == 3) {
						offset.pos.set(-100, (interior_size.y / 2 + sprite.size.x / 2) *  1);
						offset.rotation = 90;
					}

					colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET] = offset;

					engine_physical += colliders;

					engine_physical.get<components::fixtures>().set_owner_body(front);
					engine_physical.add_standard_components();
					front.add_sub_entity(engine_physical);

				const vec2 engine_size = get_resource_manager().find(assets::texture_id::TRUCK_ENGINE)->img.get_size();

				{
					messages::create_particle_effect effect;
					effect.place_of_birth = engine_physical.logic_transform();
					
					if (i == 0 || i == 1) {
						effect.place_of_birth.rotation += 180;
					}
					
					effect.input.effect = assets::particle_effect_id::ENGINE_PARTICLES;
					effect.subject = front;
					effect.input.modifier.colorize = cyan;
					effect.input.modifier.scale_amounts = 6.7f;
					effect.input.modifier.scale_lifetimes = 0.45f;
					effect.input.delete_entity_after_effect_lifetime = false;

					const auto engine = particles_existence_system().create_particle_effect_entity(world, effect);

					auto& existence = engine.get<components::particles_existence>();
					existence.distribute_within_segment_of_length = engine_size.y;

					engine.add_standard_components();
					engine_physical.add_sub_entity(engine);		
					if (i == 0) {
						front.get<components::car>().acceleration_engine[i] = engine;
					}
					if (i == 1) {
						front.get<components::car>().acceleration_engine[i] = engine;
					}
					if (i == 2) {
						front.get<components::car>().left_engine = engine;
					}
					if (i == 3) {
						front.get<components::car>().right_engine = engine;
					}
					components::particles_existence::deactivate(engine);
				}

			}

			{
				components::sound_existence::effect_input in;
				in.effect = assets::sound_buffer_id::ENGINE;
				in.modifier.repetitions = -1;
				in.delete_entity_after_effect_lifetime = false;
				const auto engine_sound = sound_existence_system().create_sound_effect_entity(world, in, spawn_transform, front);
				engine_sound.add_standard_components();
				front.add_sub_entity(engine_sound);
				front.get<components::car>().engine_sound = engine_sound;
				components::sound_existence::deactivate(engine_sound);
			}
		}

		front.add_standard_components();
		left_wheel.add_standard_components();
		interior.add_standard_components();

		return front;
	}

}