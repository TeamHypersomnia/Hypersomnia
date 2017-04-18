#include "sentience_system.h"

#include <gtest/gtest.h>

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

#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/physics/physics_scripts.h"

void sentience_system::cast_spells(const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto& metas = step.input.metas_of_assets;
	const auto now = cosmos.get_timestamp();
	const auto delta = cosmos.get_fixed_delta();

	constexpr float standard_cooldown_for_all_spells_ms = 2000.f;

	for (const auto& cast : step.input.entropy.cast_spells) {
		const auto subject = cosmos[cast.first];
		const auto spell = cast.second;

		auto& sentience = subject.get<components::sentience>();

		const auto found_spell = sentience.spells.find(spell);

		if (found_spell != sentience.spells.end()) {
			auto& spell_instance_data = (*found_spell).second;
			
			const auto spell_data = metas[spell];

			const bool can_cast_already =
				sentience.personal_electricity.value >= static_cast<meter_value_type>(spell_data.personal_electricity_required)
				&& spell_instance_data.cast_cooldown.is_ready(now, delta)
				&& sentience.cast_cooldown_for_all_spells.is_ready(now, delta)
				&& are_additional_conditions_for_casting_fulfilled(spell, subject)
			;
			
			if (can_cast_already) {
				sentience.currently_casted_spell = spell;
				sentience.time_of_last_spell_cast = now;

				sentience.personal_electricity.value -= spell_data.personal_electricity_required;
				sentience.transform_when_spell_casted = subject.get_logic_transform();

				spell_instance_data.cast_cooldown.set(
					static_cast<float>(spell_data.cooldown_ms), 
					now
				);

				sentience.cast_cooldown_for_all_spells.set(
					std::max(standard_cooldown_for_all_spells_ms, static_cast<float>(spell_data.casting_time_ms)),
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
}

void sentience_system::regenerate_values_and_advance_spell_logic(const logic_step step) const {
	const auto now = step.cosm.get_timestamp();
	const auto regeneration_frequency_in_steps = static_cast<unsigned>(1 / step.cosm.get_fixed_delta().in_seconds() * 3);
	const auto consciousness_regeneration_frequency_in_steps = static_cast<unsigned>(1 / step.cosm.get_fixed_delta().in_seconds() * 2);
	const auto pe_regeneration_frequency_in_steps = static_cast<unsigned>(1 / step.cosm.get_fixed_delta().in_seconds() * 3);
	auto& cosmos = step.cosm;
	const auto& metas = step.input.metas_of_assets;
	const auto delta = cosmos.get_fixed_delta();

	cosmos.for_each(
		processing_subjects::WITH_SENTIENCE,
		[&](const auto subject) {
			auto& sentience = subject.get<components::sentience>();

			const bool should_regenerate_values = sentience.is_conscious();

			if (should_regenerate_values) {
				if (sentience.health.is_enabled()) {
					const auto passed = (now.step - sentience.time_of_last_received_damage.step);

					if (passed > 0 && passed % regeneration_frequency_in_steps == 0) {
						sentience.health.value -= sentience.health.calculate_damage_result(-2).effective;
					}
				}

				if (sentience.consciousness.is_enabled()) {
					const auto passed = (now.step - sentience.time_of_last_exertion.step);

					if (passed > 0 && passed % consciousness_regeneration_frequency_in_steps == 0) {
						sentience.consciousness.value -= sentience.consciousness.calculate_damage_result(-2).effective;
					}
				}

				if (sentience.personal_electricity.is_enabled()) {
					const auto passed = now.step;

					if (passed > 0 && passed % pe_regeneration_frequency_in_steps == 0) {
						sentience.personal_electricity.value -= sentience.personal_electricity.calculate_damage_result(-4).effective;
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

			if (sentience.currently_casted_spell != assets::spell_id::COUNT) {
				const auto spell_data = metas[sentience.currently_casted_spell];
				const auto when_casted = sentience.time_of_last_spell_cast;

				if ((now - when_casted).in_milliseconds(delta) <= spell_data.casting_time_ms) {
					perform_spell_logic(
						step,
						sentience.currently_casted_spell,
						subject,
						sentience,
						when_casted,
						now
					);
				}
			}
		}
	);
}

void sentience_system::consume_health_event(messages::health_event h, const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto subject = cosmos[h.subject];
	const auto now = cosmos.get_timestamp();
	auto& sentience = subject.get<components::sentience>();

	switch (h.target) {
	case messages::health_event::target_type::HEALTH:
	{

		sentience.health.value -= h.effective_amount;
		ensure(sentience.health.value >= 0);
		sentience.time_of_last_received_damage = cosmos.get_timestamp();

		auto& movement = subject.get<components::movement>();
		movement.make_inert_for_ms += h.effective_amount*2;

		subject.get<components::rigid_body>()
			.apply_impulse(vec2(h.impact_velocity).set_length(static_cast<float>(h.effective_amount * 5)));

		const auto consciousness_ratio = sentience.consciousness.get_ratio();
		const auto health_ratio = sentience.health.get_ratio();

		sentience.consciousness.value = static_cast<meter_value_type>(std::min(consciousness_ratio, health_ratio) * sentience.consciousness.maximum);

		if (!sentience.health.is_positive()) {
			h.special_result = messages::health_event::result_type::DEATH;
		}
	}
		break;

	case messages::health_event::target_type::CONSCIOUSNESS: 
		sentience.consciousness.value -= h.effective_amount;

		if (!sentience.consciousness.is_positive()) {
			h.special_result = messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS;
		}
		break;

	case messages::health_event::target_type::PERSONAL_ELECTRICITY: 
		sentience.personal_electricity.value -= h.effective_amount;

		if (!sentience.personal_electricity.is_positive()) {
			h.special_result = messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION;
		}
		break;

	case messages::health_event::target_type::AIM:
		const auto punched = subject;

		if (punched[child_entity_name::CHARACTER_CROSSHAIR].alive() && punched[child_entity_name::CHARACTER_CROSSHAIR][child_entity_name::CROSSHAIR_RECOIL_BODY].alive()) {
			auto owning_crosshair_recoil = punched[child_entity_name::CHARACTER_CROSSHAIR][child_entity_name::CROSSHAIR_RECOIL_BODY];

			sentience.aimpunch.shoot_and_apply_impulse(
				owning_crosshair_recoil, 
				1 / 15.f, 
				true,
				(h.point_of_impact - punched.get_logic_transform().pos).cross(h.impact_velocity) / 100000000.f * 3.f / 25.f
			);
		}

		break;
	}

	bool knockout = false;

	if (h.special_result == messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION) {
		sentience.electric_shield = electric_shield_perk();

		sentience.personal_electricity.value = 0.f;
	}
	else if (h.special_result == messages::health_event::result_type::DEATH) {
		knockout = true;

		sentience.health.value = 0.f;
		sentience.personal_electricity.value = 0.f;
		sentience.consciousness.value = 0.f;
	}
	else if (h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
		knockout = true;

		sentience.consciousness.value = 0.f;
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
		subject.get<components::processing>().disable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
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

			const bool is_shield_enabled = s.electric_shield.timing.is_enabled(now, delta);

			auto apply_ped = [this, step, event_template, &s](const meter_value_type amount) {
				auto event = event_template;

				event.target = messages::health_event::target_type::PERSONAL_ELECTRICITY;

				const auto damaged = s.personal_electricity.calculate_damage_result(amount);
				event.effective_amount = damaged.effective;
				event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

				if (event.effective_amount != 0) {
					consume_health_event(event, step);
				}

				return damaged;
			};

			if (d.type == adverse_element_type::FORCE && s.health.is_enabled()) {
				auto event = event_template;

				auto after_shield_damage = d.amount;

				if (is_shield_enabled) {
					after_shield_damage = apply_ped(d.amount).excessive;
				}

				if (after_shield_damage > 0) {
					event.target = messages::health_event::target_type::HEALTH;

					const auto damaged = s.health.calculate_damage_result(after_shield_damage);
					event.effective_amount = damaged.effective;
					event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

					if (event.effective_amount != 0) {
						consume_health_event(event, step);
						apply_aimpunch = true;
					}
				}
			}
			
			else if (d.type == adverse_element_type::INTERFERENCE && s.consciousness.is_enabled()) {
				auto event = event_template;

				auto after_shield_damage = d.amount + static_cast<meter_value_type>(d.amount * subject.get_effective_velocity().length() / 400.f);

				if (is_shield_enabled) {
					constexpr meter_value_type absorption_by_shield_mult = 2;
					after_shield_damage = absorption_by_shield_mult * apply_ped(d.amount / absorption_by_shield_mult).excessive;
				}

				if (after_shield_damage > 0) {
					event.target = messages::health_event::target_type::CONSCIOUSNESS;

					const auto damaged = s.consciousness.calculate_damage_result(after_shield_damage);
					event.effective_amount = damaged.effective;
					event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

					if (event.effective_amount != 0) {
						consume_health_event(event, step);
						apply_aimpunch = true;
					}
				}
			}

			else if (d.type == adverse_element_type::PED && s.personal_electricity.is_enabled()) {
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
			t.get<components::sentience>().aimpunch.cooldown(step.get_delta().in_milliseconds());
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
					auto hr = sentience.health.get_ratio();
					const auto one_less_hr = 1.f - hr;

					const auto pulse_duration = static_cast<int>(1250 - 1000 * (1 - hr));
					const auto time_pulse_ratio = (timestamp_ms % pulse_duration) / static_cast<float>(pulse_duration);

					hr *= 1.f - (0.2f * time_pulse_ratio);

					if (render) {
						if (hr < 1.f) {
							render->draw_border = true;

							const auto alpha_multiplier = one_less_hr * one_less_hr * one_less_hr * one_less_hr * time_pulse_ratio;

							render->border_color = rgba(255, 0, 0, static_cast<rgba_channel>(255 * alpha_multiplier));
						}
						else {
							render->draw_border = false;
						}
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
TEST(SentienceSystem, SentienceMeters) {
	{
		cosmos c1(1);

		const auto new_ent1 = c1.create_entity("e1");
		auto& sent = new_ent1 += components::sentience();
		sent.health.set_maximum_value(100);
		sent.health.set_value(20);

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