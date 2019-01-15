#include "sentience_system.h"

#include "augs/templates/get_by_dynamic_id.h"
#include "augs/math/math.h"
#include "augs/misc/randomization.h"

#include "game/debug_drawing_settings.h"
#include "game/messages/damage_message.h"
#include "game/messages/health_event.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/components/rigid_body_component.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"
#include "game/components/render_component.h"
#include "game/components/movement_component.h"

#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/physics/physics_scripts.h"
#include "game/detail/spells/spell_logic_input.h"

#include "game/detail/gun/gun_math.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/damage_origin.hpp"
#include "game/detail/weapon_like.h"
#include "game/detail/sentience/sentience_logic.h"
#include "game/detail/crosshair_math.hpp"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/sentience/tool_getters.h"
#include "augs/templates/logically_empty.h"

damage_cause::damage_cause(const const_entity_handle& handle) {
	entity = handle;
	flavour = handle.get_flavour_id();
}

damage_origin::damage_origin(const const_entity_handle& causing_handle) : cause(causing_handle) {
	copy_sender_from(causing_handle);
}

void sentience_system::cast_spells(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto& spell_metas = cosm.get_common_significant().spells;
	const auto& clk = cosm.get_clock();	
	const auto now = clk.now;
	const auto dt = clk.dt;

	constexpr float standard_cooldown_for_all_spells_ms = 2000.f;

	for (const auto& players : step.get_entropy().players) {
		const auto subject = cosm[players.first];
		const auto spell = players.second.cast_spell;

		if (subject.dead()) {
			continue;
		}

		if (!spell.is_set()) {
			continue;
		}

		auto& sentience = subject.get<components::sentience>();

		if (!sentience.is_conscious()) {
			continue;
		}

		if (subject.is_frozen()) {
			if (!spell.is<haste>()) {
				/* Make exception for a haste spell. */
				continue;
			}
		}

		spell.dispatch(
			[&](auto s) {
				if (sentience.learnt_spells[spell.get_index()]) {
					using S = decltype(s);
					auto& spell_instance_data = std::get<instance_of<S>>(sentience.spells);

					auto& personal_electricity = sentience.get<personal_electricity_meter_instance>();

					const auto& meta = std::get<S>(spell_metas);
					const auto common = meta.common;

					const bool can_cast_already =
						personal_electricity.value >= static_cast<meter_value_type>(common.personal_electricity_required)
						&& spell_instance_data.cast_cooldown.is_ready(clk)
						&& sentience.cast_cooldown_for_all_spells.is_ready(clk)
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
						if ((now - sentience.time_of_last_exhausted_cast).in_milliseconds(dt) >= 150.f) {
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
	auto& cosm = step.get_cosmos();
	const auto delta = cosm.get_fixed_delta();

	auto make_interval_in_steps = [delta](const auto& m) {
		return static_cast<unsigned>(1 / delta.in_milliseconds() * m.regeneration_interval_ms);
	};

	cosm.for_each_having<components::sentience>(
		[&](const auto& subject) {
			const auto& sentience_def = subject.template get<invariants::sentience>();
			components::sentience& sentience = subject.template get<components::sentience>();

			auto& health = sentience.get<health_meter_instance>();
			auto& consciousness = sentience.get<consciousness_meter_instance>();
			auto& personal_electricity = sentience.get<personal_electricity_meter_instance>();

			const bool should_regenerate_values = sentience.is_conscious();

			if (should_regenerate_values) {
				if (health.is_enabled()) {
					const auto passed = (now.step - sentience.time_of_last_received_damage.step);

					if (passed > 0 && passed % make_interval_in_steps(health) == 0) {
						health.value -= health.calc_damage_result(-health.regeneration_unit).effective;
					}
				}

				if (consciousness.is_enabled()) {
					const auto passed = (now.step - sentience.time_of_last_exertion.step);
					const auto passed_ms = (now - sentience.time_of_last_exertion).in_milliseconds(delta);

					if (passed_ms >= sentience_def.exertion_cooldown_for_cp_regen_ms && passed % make_interval_in_steps(consciousness) == 0) {
						consciousness.value -= consciousness.calc_damage_result(-consciousness.regeneration_unit).effective;
					}
				}

				if (personal_electricity.is_enabled()) {
					const auto passed = now.step;

					if (passed > 0 && passed % make_interval_in_steps(personal_electricity) == 0) {
						personal_electricity.value -= personal_electricity.calc_damage_result(-personal_electricity.regeneration_unit).effective;
					}
				}
			}

			const auto since_last_shake = (now - sentience.time_of_last_shake);
			const auto shake_amount = (sentience.shake.duration_ms - since_last_shake.in_milliseconds(delta)) / sentience.shake.duration_ms;

			if (shake_amount > 0.f) {
				const auto shake_mult = shake_amount * shake_amount * sentience.shake.mult * sentience_def.shake_mult;

				auto& rng = step.step_rng;
				impulse_input in;

				in.linear = shake_mult * rng.template random_point_in_unit_circle<real32>();
				subject.apply_crosshair_recoil(in);
			}
			else {
				sentience.shake.mult = 1.f;
			}

			if (sentience.is_spell_being_cast()) {
				sentience.currently_casted_spell.dispatch(
					[&](auto s) {
						using S = decltype(s);
						using I = instance_of<S>;

						auto& spell = std::get<I>(sentience.spells);

						const auto& spell_meta = std::get<S>(cosm.get_common_significant().spells);
						const auto spell_logic_duration_ms = spell_meta.get_spell_logic_duration_ms();
						
						const auto when_casted = sentience.time_of_last_spell_cast;

						if ((now - when_casted).in_milliseconds(delta) <= spell_logic_duration_ms) {
							const auto input = spell_logic_input {
								step,
								subject,
								sentience,
								when_casted,
								now,

								sentience.currently_casted_spell
							};

							spell.perform_logic(input);
						}
					}
				);
			}
		}
	);
}

static void handle_special_result(const logic_step step, const messages::health_event& h) {
	auto& cosm = step.get_cosmos();
	const auto impact_dir = vec2(h.impact_velocity).normalize();

	const auto subject = cosm[h.subject];
	auto& sentience = subject.get<components::sentience>();

	auto& health = sentience.get<health_meter_instance>();
	auto& consciousness = sentience.get<consciousness_meter_instance>();
	auto& personal_electricity = sentience.get<personal_electricity_meter_instance>();

	const auto& origin = h.origin;

	auto knockout = [&]() {
		perform_knockout(subject, step, impact_dir, origin);
	};

	if (h.special_result == messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION) {
		sentience.get<electric_shield_perk_instance>() = electric_shield_perk_instance();

		personal_electricity.value = 0.f;

		if (const auto active_absorption = ::find_active_pe_absorption(subject)) {
			step.post_message(messages::queue_deletion(active_absorption->second));
		}
	}
	else if (h.special_result == messages::health_event::result_type::DEATH) {
		health.value = 0.f;
		personal_electricity.value = 0.f;
		consciousness.value = 0.f;
		knockout();
	}
	else if (h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
		consciousness.value = 0.f;
		//knockout();
	}
}

void sentience_system::process_and_post_health_event(messages::health_event event, const logic_step step) const {
	step.post_message(process_health_event(event, step));
}

void sentience_system::process_special_results_of_health_events(const logic_step step) const {
	const auto& events = step.template get_queue<messages::health_event>();

	for (const auto& h : events) {
		handle_special_result(step, h);
	}
}

messages::health_event sentience_system::process_health_event(messages::health_event h, const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto subject = cosm[h.subject];
	auto& sentience = subject.get<components::sentience>();
	const auto& sentience_def = subject.get<invariants::sentience>();
	auto& health = sentience.get<health_meter_instance>();
	auto& consciousness = sentience.get<consciousness_meter_instance>();
	auto& personal_electricity = sentience.get<personal_electricity_meter_instance>();
	const auto& origin = h.origin;
	const bool was_conscious = health.value > 0.f; //&& consciousness.value > 0.f;

	auto contribute_to_damage = [&](const auto contributed_amount) {
		const auto inflicting_capability = origin.get_guilty_of_damaging(subject);

		auto& owners = sentience.damage_owners;
		bool found = false;

		for (auto& o : owners) {
			if (o.who == inflicting_capability) {
				o.amount += contributed_amount;
				found = true;
			}
		}

		if (!found) {
			const auto new_one = damage_owner { inflicting_capability, contributed_amount };

			if (owners.size() == owners.max_size()) {
				if (owners.back() < new_one) {
					owners.back() = new_one;
				}
			}
			else {
				owners.push_back(new_one);
			}
		}

		sort_range(owners);
	};

	auto allow_special_result = [&]() {
		const auto disable_knockouts = step.get_settings().disable_knockouts;

		if (disable_knockouts.is_set() && subject && disable_knockouts == subject) {
			step.result.state_inconsistent = true;

			return false;
		}

		return true;
	};

	switch (h.target) {
		case messages::health_event::target_type::HEALTH: {
			const auto amount = h.effective_amount;

			contribute_to_damage(amount);
			health.value -= amount;

			ensure_geq(health.value, 0);

			sentience.time_of_last_received_damage = cosm.get_timestamp();

			auto& movement = subject.get<components::movement>();
			movement.const_inertia_ms += h.effective_amount * sentience_def.const_inertia_damage_ratio;
			//movement.linear_inertia_ms += h.effective_amount * sentience_def.linear_inertia_damage_ratio;

			const auto consciousness_ratio = consciousness.get_ratio();
			const auto health_ratio = health.get_ratio();

			const auto prev_consciousness = consciousness.value;
			consciousness.value = static_cast<meter_value_type>(std::min(consciousness_ratio, health_ratio) * consciousness.maximum);

			if (!health.is_positive()) {
				if (allow_special_result()) {
					h.special_result = messages::health_event::result_type::DEATH;
					h.was_conscious = was_conscious;
				}
				else {
					/* Assume previous consciousness so that the running ability is unimpaired */
					consciousness.value = prev_consciousness;
				}
			}

			break;
		}

		case messages::health_event::target_type::CONSCIOUSNESS: {
			const auto amount = h.effective_amount;
			contribute_to_damage(amount / 5);
			consciousness.value -= amount;

			if (!consciousness.is_positive()) {
				if (allow_special_result()) {
					h.special_result = messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS;
				}
			}

			break;
		}

		case messages::health_event::target_type::PERSONAL_ELECTRICITY: {
			const auto amount = h.effective_amount;
			personal_electricity.value -= amount;
			contribute_to_damage(amount / 8);

			if (!personal_electricity.is_positive()) {
				h.special_result = messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION;
			}

			break;
		}

		case messages::health_event::target_type::INVALID: {
			break;
		}
	}

	handle_special_result(step, h);

	return h;
}

void sentience_system::apply_damage_and_generate_health_events(const logic_step step) const {
	const auto& damages = step.get_queue<messages::damage_message>();
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto now = clk.now;

	for (const auto& d : damages) {
		const auto subject = cosm[d.subject];
		const auto& def = d.damage;
		const auto& amount = def.base;

		auto apply_impact_impulse = [&]() {
			auto considered_impulse = def.impact_impulse;

			if (considered_impulse > 0.f) {
				if (sentient_and_vulnerable(subject)) {
					considered_impulse *= def.impulse_multiplier_against_sentience;
				}

				const auto subject_of_impact = subject.get_owner_of_colliders().template get<components::rigid_body>();
				const auto subject_of_impact_mass_pos = subject_of_impact.get_mass_position(); 

				const auto impact = vec2(d.impact_velocity).set_length(considered_impulse);

				subject_of_impact.apply_impulse(impact, d.point_of_impact - subject_of_impact_mass_pos);
			}
		};

		auto* const sentience = subject.find<components::sentience>();

		messages::health_event event_template;
		event_template.subject = d.subject;
		event_template.point_of_impact = d.point_of_impact;
		event_template.impact_velocity = d.impact_velocity;
		event_template.effective_amount = 0;
		event_template.special_result = messages::health_event::result_type::NONE;
		event_template.origin = d.origin;
		
		auto process_and_post_health = [&](const auto& event) {
			process_and_post_health_event(event, step);
		};

		if (sentience) {
			const auto& s = *sentience;

			auto& health = s.get<health_meter_instance>();
			auto& consciousness = s.get<consciousness_meter_instance>();
			auto& personal_electricity = s.get<personal_electricity_meter_instance>();

			const auto absorption = ::find_active_pe_absorption(subject);
			const bool is_shield_enabled = logically_set(absorption);

			auto apply_ped = [event_template, &personal_electricity, &process_and_post_health](const meter_value_type amount) {
				auto event = event_template;

				event.target = messages::health_event::target_type::PERSONAL_ELECTRICITY;

				const auto damaged = personal_electricity.calc_damage_result(amount);
				event.effective_amount = damaged.effective;
				event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

				if (event.effective_amount != 0) {
					process_and_post_health(event);
				}

				return damaged;
			};

			if (d.type == adverse_element_type::FORCE && health.is_enabled()) {
				auto event = event_template;

				auto after_shield_damage = amount;

				if (is_shield_enabled) {
					const auto mult = std::max(0.01f, absorption->first.hp);
					after_shield_damage = apply_ped(amount / mult).excessive * mult;
				}

				if (after_shield_damage > 0) {
					event.target = messages::health_event::target_type::HEALTH;

					const auto damaged = health.calc_damage_result(after_shield_damage);
					event.effective_amount = damaged.effective;
					event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

					if (event.effective_amount != 0) {
						process_and_post_health(event);
					}
				}
			}
			
			else if (d.type == adverse_element_type::INTERFERENCE && consciousness.is_enabled()) {
				auto event = event_template;

				auto after_shield_damage = amount;

				if (is_shield_enabled) {
					const auto mult = std::max(0.01f, absorption->first.cp);
					after_shield_damage = apply_ped(amount / mult).excessive * mult;
				}

				if (after_shield_damage > 0) {
					event.target = messages::health_event::target_type::CONSCIOUSNESS;

					const auto damaged = consciousness.calc_damage_result(after_shield_damage);
					event.effective_amount = damaged.effective;
					event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

					if (event.effective_amount != 0) {
						process_and_post_health(event);
					}
				}
			}

			else if (d.type == adverse_element_type::PED && personal_electricity.is_enabled()) {
				if (is_shield_enabled) {
					apply_ped(static_cast<meter_value_type>(amount * 2.5));
				}
				else {
					apply_ped(amount);
				}
			}
		}

		const auto owning_capability = subject.get_owning_transfer_capability();

		if (d.damage.shake.any()) {
			if (auto* const head = subject.find<components::head>()) {
				if (const auto* const crosshair = subject.find_crosshair()) {
					const auto recoil_amount = crosshair->recoil.rotation;
					const auto recoil_dir = augs::sgn(recoil_amount);
					const auto considered_amount = (recoil_dir == 0 ? 1 : recoil_dir) * std::min(1.f, std::max(std::abs(recoil_amount), 0.2f));

					const auto& head_def = subject.get<invariants::head>();

					head->shake_rotation_amount += considered_amount * head_def.impulse_mult_on_shake;
				}
			}

			auto apply_shake = [&](const auto& to_whom) {
				to_whom.template dispatch_on_having_all<invariants::sentience>([&](const auto& typed_victim) {
					const auto& sentience_def = typed_victim.template get<invariants::sentience>();
					auto& sentience = typed_victim.template get<components::sentience>();

					const auto mult = sentience_def.aimpunch_mult;

					auto considered_shake = d.damage.shake;
					considered_shake *= mult;
					considered_shake.apply(now, sentience);
				});
			};

			if (sentience) {
				apply_shake(subject);
			}
			else if (owning_capability) {
				apply_shake(owning_capability);
			}
		}

		const bool held_item = sentience == nullptr && owning_capability;

		if (!held_item) {
			apply_impact_impulse();
		}
	}
}

void sentience_system::cooldown_aimpunches(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto dt = cosm.get_fixed_delta();

	cosm.for_each_having<components::head>(
		[&](const auto typed_handle) {
			const auto& head_def = typed_handle.template get<invariants::head>();
			auto& head = typed_handle.template get<components::head>();

			{
				auto& a = head.shake_rotation_amount;
				a = augs::damp(a, dt.in_seconds(), head_def.shake_rotation_damping);
			}
		}
	);
}

void sentience_system::rotate_towards_crosshairs_and_driven_vehicles(const logic_step step) const {
	auto debug_line_drawer = [](const rgba col, const vec2 a, const vec2 b){
		if (DEBUG_DRAWING.draw_colinearization) {
			DEBUG_LOGIC_STEP_LINES.emplace_back(col, a, b);
		}
	};

	auto& cosm = step.get_cosmos();

	cosm.for_each_having<components::sentience>(
		[&](const auto& subject) {
			auto& sentience = subject.template get<components::sentience>();

			if (!sentience.is_conscious()) {
				return;
			}

			const auto& fighter = subject.template get<components::melee_fighter>();
			const auto melee_state = fighter.state;

			if (melee_state == melee_fighter_state::IN_ACTION) {
				if (fighter.action == weapon_action_type::PRIMARY) {
					return;
				}
			}

			const auto subject_transform = subject.get_logic_transform();
			
			std::optional<float> requested_angle;

			if (const auto crosshair = subject.find_crosshair()) {
				const auto items = subject.get_wielded_items();

				auto& base_offset = crosshair->base_offset;
				const auto prev_base_offset = base_offset;

				const bool override_base_offset = 
					melee_state == melee_fighter_state::IN_ACTION
					&& fighter.action == weapon_action_type::SECONDARY
				;

				if (override_base_offset) {
					base_offset = fighter.overridden_crosshair_base_offset;
				}

				const auto target_transform = subject.get_world_crosshair_transform();

				if (override_base_offset) {
					base_offset = prev_base_offset;
				}

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
					const auto subject_item = cosm[items[0]];

					if (const auto* const maybe_gun_def = subject_item.template find<invariants::gun>()) {
						const auto rifle_transform = subject_item.get_logic_transform();
						auto barrel_center = calc_barrel_center(subject_item, rifle_transform);
						auto muzzle = calc_muzzle_transform(subject_item, rifle_transform).pos;
						const auto mc = subject_transform.pos;

						barrel_center.rotate(-subject_transform.rotation, mc);
						muzzle.rotate(-subject_transform.rotation, mc);

						if (/* centers_apart */ !mc.compare_abs(barrel_center)) {
							requested_angle = colinearize_AB_with_C(mc, barrel_center, muzzle, target_transform.pos, debug_line_drawer);
						}
					}
					else if (is_weapon_like(subject_item)) {
						auto throwable_transform = subject_item.get_logic_transform();
						auto throwable_target_vector = throwable_transform.pos + throwable_transform.get_direction();

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
				if (const auto vehicle = cosm[driver->owned_vehicle]) {
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