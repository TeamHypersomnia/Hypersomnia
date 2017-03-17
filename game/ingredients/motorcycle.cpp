#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/components/position_copying_component.h"
#include "game/systems_stateless/particles_existence_system.h"
#include "game/systems_stateless/sound_existence_system.h"
#include "game/components/particles_existence_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/animation_component.h"
#include "game/components/animation_response_component.h"
#include "game/components/car_component.h"
#include "game/components/trigger_component.h"
#include "game/components/name_component.h"

#include "game/enums/filters.h"

namespace prefabs {
	entity_handle create_motorcycle(cosmos& world, const components::transform& spawn_transform) {
		auto front = world.create_entity("front");
		auto left_wheel = world.create_entity("left_wheel");
		left_wheel.make_as_child_of(front);

		const auto si = world.get_si();

		name_entity(front, entity_name::JMIX114);

		{
			auto& sprite = front += components::sprite();
			auto& render = front += components::render();
			auto& car = front += components::car();
			auto& special = front += components::special_physics();
			components::physics body(si, spawn_transform);
			components::fixtures colliders;

			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(1000, 1000);
			//car.acceleration_length = 1500;
			car.acceleration_length = 1100;
			car.speed_for_pitch_unit = 2000.f;

			car.wheel_offset = vec2(35, 0);
			car.maximum_lateral_cancellation_impulse = 50;
			car.static_air_resistance = 0.0006f;
			car.dynamic_air_resistance = 0.00009f;
			car.static_damping = 2.f;
			car.dynamic_damping = 0.2f;
			car.angular_damping_while_hand_braking = 0.0f;
			car.angular_damping = 0.0f;
			//car.maximum_speed_with_static_air_resistance = 1000;
			car.maximum_speed_with_static_air_resistance = 800;
			car.maximum_speed_with_static_damping = 0.f;
			car.braking_damping = 2.f;
			
			car.minimum_speed_for_maneuverability_decrease = -1;
			car.maneuverability_decrease_multiplier = 0.00006f;

			car.lateral_impulse_multiplier = 0.3f;
			car.braking_angular_damping = 16.f;

			sprite.set(assets::game_image_id::JMIX114);
			render.layer = render_layer::CAR_INTERIOR;

			body.linear_damping = 0.4f;
			body.angular_damping = 2.f;

			auto& info = colliders.new_collider();
			info.shape.from_renderable(front);

			info.filter = filters::see_through_dynamic_object();
			info.density = 1.5f;
			info.restitution = 0.3f;
			colliders.can_driver_shoot_through = true;

			car.angular_air_resistance = 0.55f;
			car.angular_air_resistance_while_hand_braking = 0.05f;

			front += body;
			front += colliders;
			front.get<components::fixtures>().set_owner_body(front);
		}

		//{
		//	auto& sprite = interior += components::sprite();
		//	auto& render = interior += components::render();
		//	components::fixtures colliders;
		//
		//	render.layer = render_layer::CAR_WHEEL;
		//
		//	sprite.set(assets::game_image_id::MOTORCYCLE_INSIDE);
		//
		//	auto& info = colliders.new_collider();
		//	info.shape.from_renderable(interior);
		//	info.density = 0.6f;
		//	colliders.disable_standard_collision_resolution = true;
		//	info.filter = filters::see_through_dynamic_object();
		//	vec2 offset((front.get<components::sprite>().size.x / 2 + sprite.size.x / 2 - 1) * -1, 0);
		//	colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
		//	offset = front.get<components::sprite>().size + sprite.size;
		//
		//	interior += colliders;
		//	interior.get<components::fixtures>().set_owner_body(front);
		//}

		{
			auto& sprite = left_wheel += components::sprite();
			auto& render = left_wheel += components::render();
			auto& trigger = left_wheel += components::trigger();
			components::fixtures colliders;

			trigger.entity_to_be_notified = front;
			trigger.react_to_collision_detectors = false;
			trigger.react_to_query_detectors = true;

			render.layer = render_layer::CAR_WHEEL;

			sprite.set(assets::game_image_id::CAR_INSIDE, rgba(255, 255,255, 0));
			sprite.size.set(40, 20);

			auto& info = colliders.new_collider();

			info.shape.from_renderable(left_wheel);
			info.density = 0.6f;
			info.filter = filters::trigger();
			info.sensor = true;
			vec2 offset(0, 0);
			//((front.get<components::sprite>().size.x / 2 + sprite.size.x / 2) *  -1, 0);
			colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
			
			left_wheel += colliders;
			left_wheel.get<components::fixtures>().set_owner_body(front);
		}


		components::transform engine_transforms[4] = {
			{ vec2(-front.get<components::sprite>().size.x / 2 + 10, 0), 180 },
			{ vec2(front.get<components::sprite>().size.x / 2 - 10, 0), 0 },
			{ vec2(15, -(front.get<components::sprite>().size.y / 2 + 18)), 90 },
			{ vec2(15, front.get<components::sprite>().size.y / 2 + 18), -90 },

		};

		for(int ee = 0; ee < 4; ++ee)
		{
			messages::create_particle_effect effect;
			effect.place_of_birth = spawn_transform + engine_transforms[ee].pos.rotate(spawn_transform.rotation, vec2());
			effect.place_of_birth.rotation += engine_transforms[ee].rotation;
			effect.input.effect = assets::particle_effect_id::ENGINE_PARTICLES;
			effect.input.modifier.scale_amounts = 5.7f;
			effect.input.modifier.scale_lifetimes = 0.45f;
			//effect.input.displace_source_position_within_radius = 10.f;
			//effect.input.single_displacement_duration_ms.set(400.f, 1500.f);
			effect.subject = front;
			effect.input.modifier.colorize = cyan;
			effect.input.delete_entity_after_effect_lifetime = false;

			const auto engine_particles = particles_existence_system().create_particle_effect_entity(world, effect);

			auto& existence = engine_particles.get<components::particles_existence>();
			existence.distribute_within_segment_of_length = 35.f * 0.8f;

			engine_particles.add_standard_components();

			if (ee == 0) {
				front.get<components::car>().acceleration_engine[0].particles = engine_particles;
			}
			if (ee == 1) {
				front.get<components::car>().deceleration_engine[0].particles = engine_particles;
			}
			if (ee == 2) {
				front.get<components::car>().left_engine.particles = engine_particles;
			}
			if (ee == 3) {
				front.get<components::car>().right_engine.particles = engine_particles;
			}

			components::particles_existence::deactivate(engine_particles);

		}

		{
			sound_effect_input in;
			in.effect = assets::sound_buffer_id::ENGINE;
			in.modifier.repetitions = -1;
			in.delete_entity_after_effect_lifetime = false;
			const auto engine_sound = sound_existence_system().create_sound_effect_entity(world, in, spawn_transform, front);
			engine_sound.add_standard_components();
			front.get<components::car>().engine_sound = engine_sound;
			components::sound_existence::deactivate(engine_sound);
		}

		front.add_standard_components();
		left_wheel.add_standard_components();

		return front;
	}

}