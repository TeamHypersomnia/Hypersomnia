#include "game/detail/sentience/sentience_logic.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/stateless_systems/driver_system.h"
#include "game/detail/inventory/drop_from_all_slots.h"
#include "game/cosmos/create_entity.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/detail/explosive/detonate.h"

#include "augs/log.h"
void handle_corpse_damage(
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def
) {
	auto& health = sentience.get<health_meter_instance>();
	auto& when_ignited = sentience.when_corpse_catched_fire;

	const auto& cosm = subject.get_cosmos();
	const auto now = cosm.get_timestamp();

	auto ignite_corpse = [&]() {
		when_ignited = now;

		{
			const auto& effect = sentience_def.corpse_catch_fire_particles;

			effect.start(
				step,
				particle_effect_start_input::orbit_local(subject, { vec2::zero, 180 } ),
				never_predictable_v
			);
		}

		{
			const auto& effect = sentience_def.corpse_catch_fire_sound;

			effect.start(
				step,
				sound_effect_start_input::at_listener(subject),
				never_predictable_v
			);
		}
	};

	const auto damage_past_breaking_point = -health.value - sentience_def.damage_required_for_corpse_explosion;
	const bool is_ignited = when_ignited.was_set();

	if (damage_past_breaking_point > 0) {
		if (!is_ignited) {
			ignite_corpse();
		}
	}
}

void handle_corpse_detonation(
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def
) {
	if (sentience.has_exploded) {
		return;
	}

	const auto& cosm = subject.get_cosmos();
	const auto& clk = cosm.get_clock();

	auto explode_corpse = [&]() {
		::detonate({
			step,
			subject,
			sentience_def.corpse_explosion,
			subject.get_logic_transform()
		}, false);

		sentience.has_exploded = true;

		/* This will cause the corpse to disappear */
		subject.infer_colliders_from_scratch();
	};

	const auto& when_ignited = sentience.when_corpse_catched_fire;
	const auto& health = sentience.get<health_meter_instance>();
	const auto damage_past_breaking_point = -health.value - sentience_def.damage_required_for_corpse_explosion;
	const bool is_ignited = when_ignited.was_set();

	if (is_ignited) {
		const auto secs_simulated_by_damaging = (damage_past_breaking_point * 2) / 1000.f;
		const auto passed_secs = clk.get_passed_secs(when_ignited);

		if (passed_secs + secs_simulated_by_damaging >= sentience_def.corpse_burning_seconds) {
			explode_corpse();
		}
	}
}

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin
) {
	auto& cosm = step.get_cosmos(); 

	const auto subject = cosm[subject_id];

	if (subject.dead()) {
		return;
	}

	subject.dispatch_on_having_all<invariants::sentience>([&](const auto& typed_subject) {
		auto& sentience = typed_subject.template get<components::sentience>();
		auto& sentience_def = typed_subject.template get<invariants::sentience>();
		
		if (typed_subject.template get<components::item_slot_transfers>().allow_drop_and_pick) {
			if (const auto* const container = typed_subject.template find<invariants::container>()) {
				drop_from_all_slots(*container, entity_handle(typed_subject), sentience_def.drop_impulse_on_knockout, step);
			}
		}
		else {
			::queue_delete_all_owned_items(step, typed_subject);
		}

		if (const auto* const driver = typed_subject.template find<components::driver>();
			driver != nullptr && cosm[driver->owned_vehicle].alive()
		) {
			driver_system().release_car_ownership(typed_subject);
		}

		impulse_input knockout_impulse;
		knockout_impulse.linear = direction;
		knockout_impulse.angular = 1.f;

		const auto knocked_out_body = typed_subject.template get<components::rigid_body>();
		knocked_out_body.apply(knockout_impulse * sentience_def.knockout_impulse);

		{
			auto& special_physics = typed_subject.get_special_physics();

			const auto disable_collision_for_ms = 300;

			special_physics.dropped_or_created_cooldown.set(
				disable_collision_for_ms,
				cosm.get_timestamp()
			);

			special_physics.during_cooldown_ignore_collision_with = origin.sender.capability_of_sender;
		}

		sentience.when_knocked_out = cosm.get_timestamp();
		sentience.knockout_origin = origin;

		if (sentience.is_dead() && origin.circumstances.headshot) {
			const auto head_transform = typed_subject.get_logic_transform();
			const auto head_velocity = direction * sentience_def.base_detached_head_speed;
			const auto typed_subject_id = typed_subject.get_id();
			const auto head_effect = sentience_def.detached_head_particles;

			auto spawn_detached_body_part = [&](const auto& flavour) {
				cosmic::queue_create_entity(
					step,
					flavour,
					[head_transform, head_velocity, typed_subject_id](const auto& typed_entity, auto&) {
						typed_entity.set_logic_transform(head_transform);

						const auto& rigid_body = typed_entity.template get<components::rigid_body>();

						rigid_body.set_velocity(head_velocity);
						rigid_body.set_angular_velocity(7200.f);
						rigid_body.get_special().during_cooldown_ignore_collision_with = typed_subject_id;
					},

					[head_effect, typed_subject_id](const auto& typed_entity, const logic_step step) {
						if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
							typed_subject.template get<components::sentience>().detached.head = typed_entity;
						}

						const auto predictability = 
							step.get_settings().effect_prediction.predict_death_particles 
							? always_predictable_v
							: never_predictable_v
						;

						head_effect.start(
							step,
							particle_effect_start_input::orbit_local(typed_entity, { vec2::zero, 180 } ),
							predictability
						);

						head_effect.start(
							step,
							particle_effect_start_input::orbit_local(typed_subject_id, { vec2::zero, 180 } ),
							predictability
						);
					}
				);
			};

			spawn_detached_body_part(sentience_def.detached_flavours.head);
		}
	});
}
