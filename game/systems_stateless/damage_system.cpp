#include "damage_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/messages/create_particle_effect.h"
#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "augs/graphics/renderer.h"

#include "game/detail/inventory_utils.h"

#include "game/components/damage_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

#include "game/detail/physics_scripts.h"

using namespace augs;

void damage_system::destroy_colliding_bullets_and_send_damage(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	const auto& events = step.messages.get_queue<messages::collision_message>();
	step.messages.get_queue<messages::damage_message>().clear();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::BEGIN_CONTACT || it.one_is_sensor) 
			continue;

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

				if (impact_velocity.is_zero())
					impact_velocity = velocity(collider_handle);

				if (damage.impulse_upon_hit > 0.f)
					subject_of_impact.apply_force(vec2(impact_velocity).set_length(damage.impulse_upon_hit), it.point - subject_of_impact.get_mass_position());

				messages::damage_message damage_msg;
				damage_msg.inflictor = it.collider;
				damage_msg.subject = it.subject;
				damage_msg.amount = damage.amount;
				damage_msg.impact_velocity = impact_velocity;
				damage_msg.point_of_impact = it.point;
				step.messages.post(damage_msg);

				damage.saved_point_of_impact_before_death = it.point;

				const auto owning_capability = subject_handle.get_owning_transfer_capability();

				const bool is_victim_a_held_item = owning_capability.alive() && owning_capability != it.subject;

				if (!is_victim_a_held_item && damage.destroy_upon_damage) {
					damage.damage_charges_before_destruction--;

					// delete only once
					if (damage.damage_charges_before_destruction == 0)
						step.messages.post(messages::queue_destruction(it.collider));
				}
			}
		}
	}
}

void damage_system::destroy_outdated_bullets(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();

	for (const auto& it : cosmos.get(processing_subjects::WITH_DAMAGE)) {
		auto& damage = it.get<components::damage>();
	
		if ((damage.constrain_lifetime && damage.lifetime_ms >= damage.max_lifetime_ms) ||
			(damage.constrain_distance && damage.distance_travelled >= damage.max_distance)) {
			damage.saved_point_of_impact_before_death = position(it);
			step.messages.post(messages::queue_destruction(it));
		}

		if (damage.constrain_distance)
			damage.distance_travelled += speed(it);

		if (damage.constrain_lifetime)
			damage.lifetime_ms += static_cast<float>(delta.in_milliseconds());
	}
}