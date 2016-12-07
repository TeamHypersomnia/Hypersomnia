#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/components/position_copying_component.h"
#include "game/systems_stateless/particles_existence_system.h"
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
		auto interior = world.create_entity("interior");
		auto left_wheel = world.create_entity("left_wheel");

		front.add_sub_entity(interior);
		front.add_sub_entity(left_wheel);
		name_entity(front, entity_name::MOTORCYCLE);

		{
			auto& sprite = front += components::sprite();
			auto& render = front += components::render();
			auto& car = front += components::car();
			auto& special = front += components::special_physics();
			components::physics body(spawn_transform);
			components::fixtures colliders;

			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(1000, 1000);
			car.acceleration_length = 2800;
			
			car.wheel_offset = vec2(35, 0);
			car.maximum_lateral_cancellation_impulse = 50;
			car.static_air_resistance = 0.0006f;
			car.dynamic_air_resistance = 0.00009f;
			car.static_damping = 2.f;
			car.dynamic_damping = 0.2f;
			car.angular_damping_while_hand_braking = 0.0f;
			car.angular_damping = 0.0f;
			car.maximum_speed_with_static_air_resistance = 1000;
			car.maximum_speed_with_static_damping = 0.f;
			car.braking_damping = 2.f;
			
			car.minimum_speed_for_maneuverability_decrease = -1;
			car.maneuverability_decrease_multiplier = 0.00006f;

			car.lateral_impulse_multiplier = 0.3f;
			car.braking_angular_damping = 16.f;

			sprite.set(assets::texture_id::MOTORCYCLE_FRONT);
			render.layer = render_layer::CAR_WHEEL;

			body.linear_damping = 0.4f;
			body.angular_damping = 2.f;

			auto& info = colliders.new_collider();
			info.shape.from_renderable(front);

			info.filter = filters::see_through_dynamic_object();
			info.density = 0.6f;
			info.restitution = 0.3f;
			colliders.can_driver_shoot_through = true;

			car.angular_air_resistance = 0.55f;
			car.angular_air_resistance_while_hand_braking = 0.05f;

			front += body;
			front += colliders;
			front.get<components::fixtures>().set_owner_body(front);
		}
		
		vec2 rear_offset;

		{
			auto& sprite = interior += components::sprite();
			auto& render = interior += components::render();
			components::fixtures colliders;

			render.layer = render_layer::CAR_WHEEL;

			sprite.set(assets::texture_id::MOTORCYCLE_INSIDE);

			auto& info = colliders.new_collider();
			info.shape.from_renderable(interior);
			info.density = 0.6f;
			colliders.disable_standard_collision_resolution = true;
			info.filter = filters::see_through_dynamic_object();
			vec2 offset((front.get<components::sprite>().size.x / 2 + sprite.size.x / 2 - 1) * -1, 0);
			colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
			rear_offset = front.get<components::sprite>().size + sprite.size;

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

			sprite.set(assets::texture_id::CAR_INSIDE, augs::rgba(255, 0, 0, 255));
			sprite.size.set(20, 10);
			sprite.color.a = 0;

			auto& info = colliders.new_collider();

			info.shape.from_renderable(left_wheel);
			info.density = 0.6f;
			info.filter = filters::trigger();
			info.sensor = true;
			vec2 offset((front.get<components::sprite>().size.x / 2 + sprite.size.x / 2) *  -1, 0);
			colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
			
			left_wheel += colliders;
			left_wheel.get<components::fixtures>().set_owner_body(front);
		}

		{
			messages::create_particle_effect effect;
			effect.place_of_birth = spawn_transform;
			effect.place_of_birth.pos -= vec2(rear_offset.x - 40.f, 0).rotate(effect.place_of_birth.rotation, vec2());
			effect.place_of_birth.rotation += 180;
			effect.input.effect = assets::particle_effect_id::ENGINE_PARTICLES;
			//effect.input.randomize_position_within_radius = 10.f;
			//effect.input.single_displacement_duration_ms.set(400.f, 1500.f);
			effect.subject = front;
			effect.input.modifier.colorize = cyan;
			effect.input.delete_entity_after_effect_lifetime = false;

			const auto rear_engine = particles_existence_system().create_particle_effect_entity(world, effect);

			auto& existence = rear_engine.get<components::particles_existence>();
			existence.distribute_within_segment_of_length = interior.get<components::sprite>().size.y * 0.6;

			rear_engine.add_standard_components();
			front.add_sub_entity(rear_engine);
		}

		front.add_standard_components();
		left_wheel.add_standard_components();
		interior.add_standard_components();

		return front;
	}

}