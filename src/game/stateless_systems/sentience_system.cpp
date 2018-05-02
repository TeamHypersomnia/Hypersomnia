#include "sentience_system.h"

#include "augs/templates/get_by_dynamic_id.h"
#include "augs/math/math.h"
#include "augs/misc/randomization.h"

#include "game/debug_drawing_settings.h"
#include "game/messages/damage_message.h"
#include "game/messages/health_event.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/components/rigid_body_component.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"
#include "game/components/render_component.h"
#include "game/components/animation_component.h"
#include "game/components/movement_component.h"
#include "game/components/driver_component.h"

#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/physics/physics_scripts.h"
#include "game/detail/spells/spell_logic_input.h"

#include "game/stateless_systems/driver_system.h"

void sentience_system::cast_spells(const logic_step step) const {
	auto& cosmos = step.get_cosmos();
	const auto& spell_metas = cosmos.get_common_significant().spells;
	const auto now = cosmos.get_timestamp();
	const auto delta = cosmos.get_fixed_delta();

	constexpr float standard_cooldown_for_all_spells_ms = 2000.f;

	for (const auto& cast : step.get_entropy().cast_spells_per_entity) {
		const auto subject = cosmos[cast.first];
		const auto spell = cast.second;

		auto& sentience = subject.get<components::sentience>();

		if (!sentience.is_conscious()) {
			continue;
		}

		ensure(spell.is_set());

		get_by_dynamic_id(
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

							step.post_message(msg);
							
							sentience.time_of_last_exhausted_cast = now;
						}
					}
				}
			}
		);
	}
}

void sentience_system::regenerate_values_and_advance_spell_logic(const logic_step step) const {
	const auto now = step.get_cosmos().get_timestamp();
	auto& cosmos = step.get_cosmos();
	const auto delta = cosmos.get_fixed_delta();

	const auto regeneration_frequency_in_steps = static_cast<unsigned>(1 / delta.in_seconds() * 3);
	const auto consciousness_regeneration_frequency_in_steps = static_cast<unsigned>(1 / delta.in_seconds() * 2);
	const auto pe_regeneration_frequency_in_steps = static_cast<unsigned>(1 / delta.in_seconds() * 3);

	cosmos.for_each_having<components::sentience>(
		[&](const auto subject) {
			components::sentience& sentience = subject.template get<components::sentience>();
			auto& health = sentience.get<health_meter_instance>();
			auto& consciousness = sentience.get<consciousness_meter_instance>();
			auto& personal_electricity = sentience.get<personal_electricity_meter_instance>();

			const bool should_regenerate_values = sentience.is_conscious();

			if (should_regenerate_values) {
				if (health.is_enabled()) {
					const auto passed = (now.step - sentience.time_of_last_received_damage.step);

					if (passed > 0 && passed % regeneration_frequency_in_steps == 0) {
						health.value -= health.calc_damage_result(-2).effective;
					}
				}

				if (consciousness.is_enabled()) {
					const auto passed = (now.step - sentience.time_of_last_exertion.step);

					if (passed > 0 && passed % consciousness_regeneration_frequency_in_steps == 0) {
						consciousness.value -= consciousness.calc_damage_result(-2).effective;
					}
				}

				if (personal_electricity.is_enabled()) {
					const auto passed = now.step;

					if (passed > 0 && passed % pe_regeneration_frequency_in_steps == 0) {
						personal_electricity.value -= personal_electricity.calc_damage_result(-4).effective;
					}
				}
			}

			const auto since_last_shake = (now - sentience.time_of_last_shake);
			const auto shake_amount = (sentience.shake.duration_ms - since_last_shake.in_milliseconds(delta)) / sentience.shake.duration_ms;

			if (shake_amount > 0.f) {
				const auto shake_mult = shake_amount * shake_amount * sentience.shake.mult;

				auto rng = cosmos.get_rng_for(subject);
				impulse_input in;

				in.linear = shake_mult * rng.template random_point_in_unit_circle<real32>();
				subject.apply_crosshair_recoil(in);
			}
			else {
				sentience.shake.mult = 1.f;
			}

			if (sentience.is_spell_being_cast()) {
				get_by_dynamic_id(
					sentience.spells, 
					sentience.currently_casted_spell,
					[&](auto& spell){
						const auto spell_meta = get_meta_of(spell, cosmos.get_common_significant().spells);
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
	auto& cosmos = step.get_cosmos();
	const auto subject = cosmos[h.subject];
	auto& sentience = subject.get<components::sentience>();
	auto& sentience_def = subject.get<invariants::sentience>();
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

	case messages::health_event::target_type::INVALID:
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
		const auto* const container = subject.find<invariants::container>();

		if (container) {
			const auto& container = subject.get<invariants::container>();
			drop_from_all_slots(container, subject, sentience_def.drop_impulse_on_knockout, step);
		}

		auto& driver = subject.get<components::driver>();
		
		if(cosmos[driver.owned_vehicle].alive()) {
			driver_system().release_car_ownership(subject);
		}
		
		impulse_input knockout_impulse;
		knockout_impulse.linear = h.impact_velocity.normalize();
		knockout_impulse.angular = 1.f;

		const auto knocked_out_body = subject.get<components::rigid_body>();
		knocked_out_body.apply(knockout_impulse * sentience_def.knockout_impulse);
	}

	step.post_message(h);
}

void sentience_system::apply_damage_and_generate_health_events(const logic_step step) const {
	const auto& damages = step.get_queue<messages::damage_message>();
	auto& cosmos = step.get_cosmos();
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
		
		if (sentience) {
			const auto& s = *sentience;

			auto& health = s.get<health_meter_instance>();
			auto& consciousness = s.get<consciousness_meter_instance>();
			auto& personal_electricity = s.get<personal_electricity_meter_instance>();

			const bool is_shield_enabled = s.get<electric_shield_perk_instance>().timing.is_enabled(now, delta);

			auto apply_ped = [this, step, event_template, &personal_electricity](const meter_value_type amount) {
				auto event = event_template;

				event.target = messages::health_event::target_type::PERSONAL_ELECTRICITY;

				const auto damaged = personal_electricity.calc_damage_result(amount);
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

					const auto damaged = health.calc_damage_result(after_shield_damage);
					event.effective_amount = damaged.effective;
					event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

					if (event.effective_amount != 0) {
						consume_health_event(event, step);
					}
				}
			}
			
			else if (d.type == adverse_element_type::INTERFERENCE && consciousness.is_enabled()) {
				auto event = event_template;

				auto after_shield_damage = d.amount;

				if (is_shield_enabled) {
					constexpr meter_value_type absorption_by_shield_mult = 2;
					after_shield_damage = absorption_by_shield_mult * apply_ped(d.amount / absorption_by_shield_mult).excessive;
				}

				if (after_shield_damage > 0) {
					event.target = messages::health_event::target_type::CONSCIOUSNESS;

					const auto damaged = consciousness.calc_damage_result(after_shield_damage);
					event.effective_amount = damaged.effective;
					event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

					if (event.effective_amount != 0) {
						consume_health_event(event, step);
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

		if (d.victim_shake.any()) {
			if (sentience) {
				d.victim_shake.apply(now, *sentience);
			}
			else if (const auto owning_capability = subject.get_owning_transfer_capability()) {
				if (const auto s = owning_capability.find<components::sentience>()) {
					d.victim_shake.apply(now, *s);
				}
			}
		}
	}
}

void sentience_system::cooldown_aimpunches(const logic_step step) const {
	const auto& cosmos = step.get_cosmos();

	cosmos.for_each_having<components::sentience>(
		[&](const auto) {

		}
	);
}

void sentience_system::rotate_towards_crosshairs_and_driven_vehicles(const logic_step step) const {
	auto debug_line_drawer = [](const rgba col, const vec2 a, const vec2 b){
		DEBUG_LOGIC_STEP_LINES.emplace_back(col, a, b);
	};

	auto& cosmos = step.get_cosmos();

	cosmos.for_each_having<components::sentience>(
		[&](const auto subject) {
			components::sentience& sentience = subject.template get<components::sentience>();

			if (!sentience.is_conscious()) {
				return;
			}

			const auto subject_transform = subject.get_logic_transform();
			
			std::optional<float> requested_angle;

			if (const auto crosshair = subject.find_crosshair()) {
				const auto items = subject.get_wielded_items();

				const auto target_transform = subject.get_world_crosshair_transform();

				{
					const auto diff = target_transform.pos - subject_transform.pos;

					if (diff.is_epsilon(1.f)) {
						requested_angle = 0.f;
					}
					else {
						requested_angle = diff.degrees();
					}
				}

				if (items.size() > 0) {
					const auto subject_item = cosmos[items[0]];

					if (const auto* const maybe_gun_def = subject_item.template find<invariants::gun>()) {
						const auto& gun_def = *maybe_gun_def;

						const auto rifle_transform = subject_item.get_logic_transform();
						auto barrel_center = gun_def.calc_barrel_center(rifle_transform);
						auto muzzle = gun_def.calc_muzzle_position(rifle_transform);
						const auto mc = subject_transform.pos;

						barrel_center.rotate(-subject_transform.rotation, mc);
						muzzle.rotate(-subject_transform.rotation, mc);

						if (/* centers_apart */ !mc.compare_abs(barrel_center)) {
							requested_angle = colinearize_AB_with_C(mc, barrel_center, muzzle, target_transform.pos, debug_line_drawer);
						}
					}
					else if (subject_item.template find<components::hand_fuse>()) {
						auto throwable_transform = subject_item.get_logic_transform();
						auto throwable_target_vector = throwable_transform.pos + vec2::from_degrees(throwable_transform.rotation);

						const auto mc = subject_transform.pos;

						throwable_transform.pos.rotate(-subject_transform.rotation, mc);
						throwable_target_vector.rotate(-subject_transform.rotation, mc);

						if (/* centers_apart */ !mc.compare_abs(throwable_transform.pos)) {
							requested_angle = colinearize_AB_with_C(mc, throwable_transform.pos, throwable_target_vector, target_transform.pos, debug_line_drawer);
						}
					}
				}
			}

			if (const auto driver = subject.template find<components::driver>()) {
				if (const auto vehicle = cosmos[driver->owned_vehicle]) {
					const auto target_transform = vehicle.get_logic_transform();
					const auto diff = target_transform.pos - subject_transform.pos;
					requested_angle = diff.degrees();
				}
			}

			if (auto rigid_body = subject.template find<components::rigid_body>();
				rigid_body != nullptr && requested_angle
			) {
				rigid_body.set_transform({ rigid_body.get_position(), *requested_angle });
				rigid_body.set_angular_velocity(0);
			}
		}
	);
}