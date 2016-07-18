#include "damage_system.h"
#include "game/cosmos.h"
#include "game/entity_id.h"

#include "game/messages/create_particle_effect.h"
#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "augs/graphics/renderer.h"

#include "game/detail/inventory_utils.h"

#include "game/components/damage_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

#include "game/entity_handle.h"
#include "game/step.h"

#include "game/detail/physics_scripts.h"

using namespace augs;

void damage_system::destroy_colliding_bullets_and_send_damage(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto events = step.messages.get_queue<messages::collision_message>();
	step.messages.get_queue<messages::damage_message>().clear();

	for (auto it : events) {
		if (it.type != messages::collision_message::event_type::BEGIN_CONTACT) 
			continue;

		auto subject_handle = cosmos[it.subject];
		auto collider_handle = cosmos[it.collider];

		auto* damage = collider_handle.find<components::damage>();

		bool bullet_colliding_with_sender =
			damage && cosmos.get_handle(damage->sender).get_owner_body_entity() == subject_handle.get_owner_body_entity();

		if (!bullet_colliding_with_sender && damage && damage->damage_upon_collision && damage->damage_charges_before_destruction > 0) {
			auto& subject_of_impact = subject_handle.get_owner_body_entity().get<components::physics>();
			
			vec2 impact_velocity = damage->custom_impact_velocity;
			
			if(impact_velocity.is_zero())
				impact_velocity = velocity(collider_handle);

			if(damage->impulse_upon_hit > 0.f)
				subject_of_impact.apply_force(vec2(impact_velocity).set_length(damage->impulse_upon_hit), it.point - subject_of_impact.get_mass_position());

			messages::damage_message damage_msg;
			damage_msg.inflictor = it.collider;
			damage_msg.subject = it.subject;
			damage_msg.amount = damage->amount;
			damage_msg.impact_velocity = impact_velocity;
			damage_msg.point_of_impact = it.point;
			step.messages.post(damage_msg);

			damage->saved_point_of_impact_before_death = it.point;

			auto owning_capability = subject_handle.get_owning_transfer_capability();

			bool is_victim_a_held_item = cosmos[owning_capability].alive() && owning_capability != it.subject;

			if (!is_victim_a_held_item && damage->destroy_upon_damage) {
				damage->damage_charges_before_destruction--;
				
				// delete only once
				if(damage->damage_charges_before_destruction == 0)
					step.messages.post(messages::queue_destruction(it.collider));
			}
		}
	}
}

void damage_system::destroy_outdated_bullets(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto targets_copy = cosmos.get(processing_subjects::WITH_DAMAGE);
	for (auto it : targets_copy) {
		auto& damage = it.get<components::damage>();
	
		if ((damage.constrain_lifetime && damage.lifetime_ms >= damage.max_lifetime_ms) ||
			(damage.constrain_distance && damage.distance_travelled >= damage.max_distance)) {
			damage.saved_point_of_impact_before_death = position(it);
			step.messages.post(messages::queue_destruction(it));
		}

		damage.distance_travelled += speed(it);
		damage.lifetime_ms += static_cast<float>(cosmos.delta.in_milliseconds());
	}
}