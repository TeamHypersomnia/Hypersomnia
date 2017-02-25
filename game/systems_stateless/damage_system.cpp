#include "damage_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "augs/graphics/renderer.h"

#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/entity_scripts.h"

#include "game/components/damage_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/physics/physics_scripts.h"

#include "game/assets/sound_buffer_id.h"

#include "game/enums/filters.h"

#include "game/systems_stateless/sound_existence_system.h"

using namespace augs;

void damage_system::destroy_colliding_bullets_and_send_damage(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& events = step.transient.messages.get_queue<messages::collision_message>();
	step.transient.messages.get_queue<messages::damage_message>().clear();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::BEGIN_CONTACT || it.one_is_sensor) {
			continue;
		}

		const auto subject_handle = cosmos[it.subject];
		const auto collider_handle = cosmos[it.collider];

		if (collider_handle.has<components::damage>()) {
			auto& damage = collider_handle.get<components::damage>();
			const auto sender = cosmos[damage.sender];

			const bool bullet_colliding_with_sender = sender.get_owner_body() == subject_handle.get_owner_body();
			bool bullet_colliding_with_senders_vehicle = false;

			{
				auto* driver = sender.get_owner_body().find<components::driver>();

				if (driver) {
					bullet_colliding_with_senders_vehicle = driver->owned_vehicle == subject_handle.get_owner_body()
						&& subject_handle.get_owner_body().get<components::fixtures>().can_driver_shoot_through();

					//LOG("ownedveh: %x\n subj owner: %x\n, compo: %x, res: %x",
					//	driver->owned_vehicle, subject_handle.get_owner_body(),
					//	subject_handle.get_owner_body().get<components::fixtures>().can_driver_shoot_through(), bullet_colliding_with_senders_vehicle);
				}
			}

			if (!bullet_colliding_with_sender && !bullet_colliding_with_senders_vehicle && 
				damage.damage_upon_collision && damage.damage_charges_before_destruction > 0) {
				auto& subject_of_impact = subject_handle.get_owner_body().get<components::physics>();

				vec2 impact_velocity = damage.custom_impact_velocity;

				if (impact_velocity.is_zero()) {
					impact_velocity = velocity(collider_handle);
				}

				if (damage.impulse_upon_hit > 0.f) {
					subject_of_impact.apply_force(vec2(impact_velocity).set_length(damage.impulse_upon_hit), it.point - subject_of_impact.get_mass_position());
				}

				damage.saved_point_of_impact_before_death = it.point;

				const auto owning_capability = subject_handle.get_owning_transfer_capability();

				const bool is_victim_a_held_item = owning_capability.alive() && owning_capability != it.subject;

				messages::damage_message damage_msg;
				damage_msg.subject_b2Fixture_index = it.subject_b2Fixture_index;
				damage_msg.collider_b2Fixture_index = it.collider_b2Fixture_index;

				if (is_victim_a_held_item) {
					components::sound_existence::effect_input in;
					in.effect = assets::sound_buffer_id::BULLET_PASSES_THROUGH_HELD_ITEM;
					in.delete_entity_after_effect_lifetime = true;
					in.direct_listener = owning_capability;

					sound_existence_system().create_sound_effect_entity(cosmos, in, { it.point, 0.f }, entity_id() ).add_standard_components();
				}

				if (!is_victim_a_held_item && damage.destroy_upon_damage) {
					damage.damage_charges_before_destruction--;

					// delete only once
					if (damage.damage_charges_before_destruction == 0) {
						step.transient.messages.post(messages::queue_destruction(it.collider));
						damage_msg.inflictor_destructed = true;
					}
				}

				damage_msg.inflictor = it.collider;
				damage_msg.subject = it.subject;
				damage_msg.amount = damage.amount;
				damage_msg.impact_velocity = impact_velocity;
				damage_msg.point_of_impact = it.point;
				step.transient.messages.post(damage_msg);
			}
		}
	}
}

void damage_system::destroy_outdated_bullets(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();

	for (const auto& it : cosmos.get(processing_subjects::WITH_DAMAGE)) {
		auto& damage = it.get<components::damage>();
	
		if ((damage.constrain_lifetime && damage.lifetime_ms >= damage.max_lifetime_ms) ||
			(damage.constrain_distance && damage.distance_travelled >= damage.max_distance)) {
			damage.saved_point_of_impact_before_death = position(it);
			step.transient.messages.post(messages::queue_destruction(it));
		}

		if (damage.constrain_distance) {
			damage.distance_travelled += speed(it);
		}

		if (damage.constrain_lifetime) {
			damage.lifetime_ms += static_cast<float>(delta.in_milliseconds());
		}

		if (damage.homing_towards_hostile_strength > 0.f) {
			const auto sender_capability = cosmos[damage.sender].get_owning_transfer_capability();
			const auto sender_attitude = sender_capability.alive() && sender_capability.has<components::attitude>() ? sender_capability : cosmos[entity_id()];

			const auto particular_homing_target = cosmos[damage.particular_homing_target];
			
			const auto closest_hostile = 
				particular_homing_target.alive() ? particular_homing_target : cosmos[get_closest_hostile(it, sender_attitude, 250, filters::bullet())];

			const auto current_velocity = it.get<components::physics>().velocity();

			it.set_logic_transform({ it.get_logic_transform().pos, current_velocity.degrees() });

			if (closest_hostile.alive()) {
				vec2 dirs[] = { current_velocity.perpendicular_cw(), -current_velocity.perpendicular_cw() };

				auto homing_vector = closest_hostile.get_logic_transform().pos - it.get_logic_transform().pos;

				if (dirs[0].radians_between(homing_vector) > dirs[1].radians_between(homing_vector)) {
					std::swap(dirs[0], dirs[1]);
				}

				it.get<components::physics>().apply_force(
					dirs[0].set_length(homing_vector.length()) * damage.homing_towards_hostile_strength
				);
			}
		}
	}
}