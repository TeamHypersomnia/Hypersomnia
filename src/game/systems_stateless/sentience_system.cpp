#include "sentience_system.h"

#include "augs/templates/dynamic_dispatch.h"
#include "game/messages/damage_message.h"
#include "game/messages/health_event.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/entity_id.h"

#include "game/components/rigid_body_component.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/sentience_component.h"
#include "game/components/render_component.h"
#include "game/components/animation_component.h"
#include "game/components/movement_component.h"
#include "game/components/driver_component.h"

#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/physics/physics_scripts.h"
#include "game/detail/spells/spell_logic_input.h"

#include "game/systems_stateless/driver_system.h"

void sentience_system::cast_spells(const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto& spell_metas = step.cosm.get_global_state().spells;
	const auto now = cosmos.get_timestamp();
	const auto delta = cosmos.get_fixed_delta();

	constexpr float standard_cooldown_for_all_spells_ms = 2000.f;

	for (const auto& cast : step.input.entropy.cast_spells_per_entity) {
		const auto subject = cosmos[cast.first];
		const auto spell = cast.second;

		auto& sentience = subject.get<components::sentience>();

		ensure(spell.is_set());

		dynamic_dispatch(
			sentience.spells,
			spell,
			[&](auto& spell_instance_data){
				if (sentience.learned_spells[spell.get_index()]) {
					auto& personal_electricity = sentience.get<personal_electricity_meter_instance>();

					const auto meta = get_meta_of(spell_instance_data, spell_metas);
					const auto common = meta.common;

					const bool can_cast_already =
						personal_electricity.value >= static_cast<meter_value_type>(common.personal_electricity_required)
						&& spell_instance_data.cast_cooldown.is_ready(now, delta)
						&& sentience.cast_cooldown_for_all_spells.is_ready(now, delta)
						&& spell_instance_data.are_additional_conditions_for_casting_fulfilled(subject)
					;
					
					if (can_cast_already) {
						sentience.currently_casted_spell = spell;
						sentience.time_of_last_spell_cast = now;

						personal_electricity.value -= common.personal_electricity_required;
						sentience.transform_when_spell_casted = subject.get_logic_transform();

						spell_instance_data.cast_cooldown.set(
							static_cast<float>(common.cooldown_ms), 
							now
						);
						
						sentience.cast_cooldown_for_all_spells.set(
							std::max(standard_cooldown_for_all_spells_ms, static_cast<float>(meta.get_spell_logic_duration_ms())),
							now
						);
					}
					else {
						if ((now - sentience.time_of_last_exhausted_cast).in_milliseconds(delta) >= 150.f) {
							messages::exhausted_cast msg;
							msg.subject = subject;
							msg.transform = subject.get_logic_transform();

							step.transient.messages.post(msg);
							
							sentience.time_of_last_exhausted_cast = now;
						}
					}
				}
			}
		);
	}
}

void sentience_system::regenerate_values_and_advance_spell_logic(const logic_step step) const {
	const auto now = step.cosm.get_timestamp();
	auto& cosmos = step.cosm;
	const auto delta = cosmos.get_fixed_delta();

	const auto regeneration_frequency_in_steps = static_cast<unsigned>(1 / delta.in_seconds() * 3);
	const auto consciousness_regeneration_frequency_in_steps = static_cast<unsigned>(1 / delta.in_seconds() * 2);
	const auto pe_regeneration_frequency_in_steps = static_cast<unsigned>(1 / delta.in_seconds() * 3);
	const auto& metas = step.input.metas_of_assets;

	cosmos.for_each(
		processing_subjects::WITH_SENTIENCE,
		[&](const auto subject) {
			auto& sentience = subject.get<components::sentience>();
			auto& health = sentience.get<health_meter_instance>();
			auto& consciousness = sentience.get<consciousness_meter_instance>();
			auto& personal_electricity = sentience.get<personal_electricity_meter_instance>();

			const bool should_regenerate_values = sentience.is_conscious();

			if (should_regenerate_values) {
				if (health.is_enabled()) {
					const auto passed = (now.step - sentience.time_of_last_received_damage.step);

					if (passed > 0 && passed % regeneration_frequency_in_steps == 0) {
						health.value -= health.calculate_damage_result(-2).effective;
					}
				}

				if (consciousness.is_enabled()) {
					const auto passed = (now.step - sentience.time_of_last_exertion.step);

					if (passed > 0 && passed % consciousness_regeneration_frequency_in_steps == 0) {
						consciousness.value -= consciousness.calculate_damage_result(-2).effective;
					}
				}

				if (personal_electricity.is_enabled()) {
					const auto passed = now.step;

					if (passed > 0 && passed % pe_regeneration_frequency_in_steps == 0) {
						personal_electricity.value -= personal_electricity.calculate_damage_result(-4).effective;
					}
				}
			}

			const auto shake_mult = (sentience.shake_for_ms - (now - sentience.time_of_last_shake).in_milliseconds(delta)) / 400.f;

			if (shake_mult > 0.f) {
				const auto owning_crosshair_recoil = subject[child_entity_name::CHARACTER_CROSSHAIR][child_entity_name::CROSSHAIR_RECOIL_BODY];
				auto rng = cosmos.get_rng_for(subject);

				owning_crosshair_recoil.get<components::rigid_body>().apply_impulse(
					shake_mult *shake_mult * 100 * vec2{ rng.randval(-1.f, 1.f), rng.randval(-1.f, 1.f) });
			}

			if (sentience.is_spell_being_cast()) {
				dynamic_dispatch(
					sentience.spells, 
					sentience.currently_casted_spell,
					[&](auto& spell){
						const auto spell_meta = get_meta_of(spell, cosmos.get_global_state().spells);
						const auto spell_logic_duration_ms = spell_meta.get_spell_logic_duration_ms();
						
						const auto when_casted = sentience.time_of_last_spell_cast;

						if ((now - when_casted).in_milliseconds(delta) <= spell_logic_duration_ms) {
							const auto input = spell_logic_input {
								step,
								subject,
								sentience,
								when_casted,
								now
							};

							spell.perform_logic(input);
						}
					}
				);
			}
		}
	);
}

void sentience_system::consume_health_event(messages::health_event h, const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto subject = cosmos[h.subject];
	const auto now = cosmos.get_timestamp();
	auto& sentience = subject.get<components::sentience>();
	auto& health = sentience.get<health_meter_instance>();
	auto& consciousness = sentience.get<consciousness_meter_instance>();
	auto& personal_electricity = sentience.get<personal_electricity_meter_instance>();

	switch (h.target) {
	case messages::health_event::target_type::HEALTH:
	{

		health.value -= h.effective_amount;
		ensure(health.value >= 0);
		sentience.time_of_last_received_damage = cosmos.get_timestamp();

		auto& movement = subject.get<components::movement>();
		movement.make_inert_for_ms += h.effective_amount*2;

		subject.get<components::rigid_body>()
			.apply_impulse(vec2(h.impact_velocity).set_length(static_cast<float>(h.effective_amount * 5)));

		const auto consciousness_ratio = consciousness.get_ratio();
		const auto health_ratio = health.get_ratio();

		consciousness.value = static_cast<meter_value_type>(std::min(consciousness_ratio, health_ratio) * consciousness.maximum);

		if (!health.is_positive()) {
			h.special_result = messages::health_event::result_type::DEATH;
		}
	}
		break;

	case messages::health_event::target_type::CONSCIOUSNESS: 
		consciousness.value -= h.effective_amount;

		if (!consciousness.is_positive()) {
			h.special_result = messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS;
		}
		break;

	case messages::health_event::target_type::PERSONAL_ELECTRICITY: 
		personal_electricity.value -= h.effective_amount;

		if (!personal_electricity.is_positive()) {
			h.special_result = messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION;
		}
		break;

	case messages::health_event::target_type::AIM:
		const auto punched = subject;

		if (punched[child_entity_name::CHARACTER_CROSSHAIR].alive() && punched[child_entity_name::CHARACTER_CROSSHAIR][child_entity_name::CROSSHAIR_RECOIL_BODY].alive()) {
			auto owning_crosshair_recoil = punched[child_entity_name::CHARACTER_CROSSHAIR][child_entity_name::CROSSHAIR_RECOIL_BODY];
			auto& recoil_physics = owning_crosshair_recoil.get<components::rigid_body>();

			recoil_physics.apply_angular_impulse((h.point_of_impact - punched.get_logic_transform().pos).cross(h.impact_velocity) / 10000000.f * 3.f / 25.f);
		}

		break;
	}

	bool knockout = false;

	if (h.special_result == messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION) {
		sentience.get<electric_shield_perk_instance>() = electric_shield_perk_instance();

		personal_electricity.value = 0.f;
	}
	else if (h.special_result == messages::health_event::result_type::DEATH) {
		knockout = true;

		health.value = 0.f;
		personal_electricity.value = 0.f;
		consciousness.value = 0.f;
	}
	else if (h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
		knockout = true;

		consciousness.value = 0.f;
	}

	if (knockout) {
		const auto* const container = subject.find<components::container>();

		if (container) {
			//drop_from_all_slots(subject, step);

			const auto& container = subject.get<components::container>();

			for (const auto& s : container.slots) {
				for (const auto item_id : s.second.items_inside) {
					const auto item = cosmos[item_id];

					perform_transfer({ item, inventory_slot_id() }, step);
				}
			}
		}

		//
		//auto place_of_death = subject.get_logic_transform();
		//place_of_death.rotation = h.impact_velocity.degrees();
		//
		//corpse.set_logic_transform(step, place_of_death);
		//
		//subject.get<components::fixtures>().set_activated(false);
		//subject.get<components::position_copying>().set_target(corpse);
		//
		//corpse.add_standard_components(step);
		//
		
		//
		auto& driver = subject.get<components::driver>();
		
		if(cosmos[driver.owned_vehicle].alive()) {
			driver_system().release_car_ownership(subject);
		}
		
		driver.take_hold_of_wheel_when_touched = false;

		subject.get<components::processing>().disable_in(processing_subjects::WITH_MOVEMENT);
		subject.get<components::processing>().disable_in(processing_subjects::WITH_ROTATION_COPYING);
		resolve_dampings_of_body(subject);
		
		const auto subject_transform = subject.get_logic_transform();

		subject.get<components::rigid_body>().apply_impulse(
			h.impact_velocity.set_length(850) 
			//vec2().set_from_degrees(subject_transform.rotation) * 70
		);


		subject.get<components::rigid_body>().apply_angular_impulse(
			80.f
		);
	}

	step.transient.messages.post(h);
}

void sentience_system::apply_damage_and_generate_health_events(const logic_step step) const {
	const auto& damages = step.transient.messages.get_queue<messages::damage_message>();
	auto& cosmos = step.cosm;
	const auto now = cosmos.get_timestamp();
	const auto delta = cosmos.get_fixed_delta();

	for (const auto& d : damages) {
		const auto subject = cosmos[d.subject];

		auto* const sentience = subject.find<components::sentience>();

		messages::health_event event_template;
		event_template.subject = d.subject;
		event_template.point_of_impact = d.point_of_impact;
		event_template.impact_velocity = d.impact_velocity;
		event_template.effective_amount = 0;
		event_template.special_result = messages::health_event::result_type::NONE;
		
		bool apply_aimpunch = false;

		if (sentience) {
			const auto& s = *sentience;

			auto& health = s.get<health_meter_instance>();
			auto& consciousness = s.get<consciousness_meter_instance>();
			auto& personal_electricity = s.get<personal_electricity_meter_instance>();

			const bool is_shield_enabled = s.get<electric_shield_perk_instance>().timing.is_enabled(now, delta);

			auto apply_ped = [this, step, event_template, &personal_electricity](const meter_value_type amount) {
				auto event = event_template;

				event.target = messages::health_event::target_type::PERSONAL_ELECTRICITY;

				const auto damaged = personal_electricity.calculate_damage_result(amount);
				event.effective_amount = damaged.effective;
				event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

				if (event.effective_amount != 0) {
					consume_health_event(event, step);
				}

				return damaged;
			};

			if (d.type == adverse_element_type::FORCE && health.is_enabled()) {
				auto event = event_template;

				auto after_shield_damage = d.amount;

				if (is_shield_enabled) {
					after_shield_damage = apply_ped(d.amount).excessive;
				}

				if (after_shield_damage > 0) {
					event.target = messages::health_event::target_type::HEALTH;

					const auto damaged = health.calculate_damage_result(after_shield_damage);
					event.effective_amount = damaged.effective;
					event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

					if (event.effective_amount != 0) {
						consume_health_event(event, step);
						apply_aimpunch = true;
					}
				}
			}
			
			else if (d.type == adverse_element_type::INTERFERENCE && consciousness.is_enabled()) {
				auto event = event_template;

				auto after_shield_damage = d.amount + static_cast<meter_value_type>(d.amount * subject.get_effective_velocity().length() / 400.f);

				if (is_shield_enabled) {
					constexpr meter_value_type absorption_by_shield_mult = 2;
					after_shield_damage = absorption_by_shield_mult * apply_ped(d.amount / absorption_by_shield_mult).excessive;
				}

				if (after_shield_damage > 0) {
					event.target = messages::health_event::target_type::CONSCIOUSNESS;

					const auto damaged = consciousness.calculate_damage_result(after_shield_damage);
					event.effective_amount = damaged.effective;
					event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

					if (event.effective_amount != 0) {
						consume_health_event(event, step);
						apply_aimpunch = true;
					}
				}
			}

			else if (d.type == adverse_element_type::PED && personal_electricity.is_enabled()) {
				if (is_shield_enabled) {
					apply_ped(static_cast<meter_value_type>(d.amount * 2.5));
				}
				else {
					apply_ped(d.amount);
				}
			}
		}

		auto aimpunch_event = event_template;
		aimpunch_event.target = messages::health_event::target_type::AIM;

		if (sentience) {
			aimpunch_event.subject = subject;
			
			if (apply_aimpunch) {
				consume_health_event(aimpunch_event, step);

				sentience->shake_for_ms = d.request_shake_for_ms + std::max(0.f, (sentience->shake_for_ms - (now - sentience->time_of_last_shake).in_milliseconds(delta)));
				sentience->time_of_last_shake = now;
			}
		}
		else {
			const auto owning_capability = subject.get_owning_transfer_capability();
			
			if (owning_capability.alive()) {
				aimpunch_event.subject = owning_capability;

				if (d.amount > 0.f) {
					consume_health_event(aimpunch_event, step);
				}
			}
		}
	}
}

void sentience_system::cooldown_aimpunches(const logic_step step) const {
	step.cosm.for_each(
		processing_subjects::WITH_SENTIENCE,
		[&](const auto t) {

		}
	);
}

void sentience_system::set_borders(const logic_step step) const {
	const auto timestamp_ms = static_cast<int>(step.cosm.get_total_time_passed_in_seconds() * 1000.0);
	
	step.cosm.for_each(
		processing_subjects::WITH_SENTIENCE,
		[&](const auto t) {
			const auto& sentience = t.get<components::sentience>();

			auto* const render = t.find<components::render>();

			if (render != nullptr) {
				if (sentience.is_conscious()) {
					const auto& health = sentience.get<health_meter_instance>();
					auto hr = health.get_ratio();
					const auto one_less_hr = 1.f - hr;

					const auto pulse_duration = static_cast<int>(1250 - 1000 * (1 - hr));
					const auto time_pulse_ratio = (timestamp_ms % pulse_duration) / static_cast<float>(pulse_duration);

					hr *= 1.f - (0.2f * time_pulse_ratio);

					if (hr < 1.f) {
						render->draw_border = true;

						const auto alpha_multiplier = one_less_hr * one_less_hr * one_less_hr * one_less_hr * time_pulse_ratio;

						render->border_color = rgba(255, 0, 0, static_cast<rgba_channel>(255 * alpha_multiplier));
					}
					else {
						render->draw_border = false;
					}
				}
				else {
					render->draw_border = false;
				}
			}
		}
	);
}
/*

#include <catch.hpp>

TEST_CASE("SentienceSystem", "SentienceMeters") {
	{
		cosmos c1(1);

		const auto new_ent1 = c1.create_entity("e1");
		auto& sent = new_ent1 += components::sentience();

		messages::damage_message msg;
		msg.amount = 40.f;

		c1.advance_deterministic_schemata(
		{},
			[](const logic_step step) {
			//step.transient.messages.post
		},
			[](auto...) {

			}
			);
	}
}*/