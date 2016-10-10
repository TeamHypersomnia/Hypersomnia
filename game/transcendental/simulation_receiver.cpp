#include "augs/templates.h"
#include "simulation_receiver.h"
#include "game/transcendental/cosmic_delta.h"
#include "augs/network/network_client.h"

#include "game/components/driver_component.h"
#include "game/components/flags_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/gun_component.h"
#include "game/components/item_component.h"

#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/detail/inventory_slot_handle.h"

void simulation_receiver::acquire_next_packaged_step(const step_packaged_for_network& step) {
	jitter_buffer.acquire_new_command(step);
}

void simulation_receiver::remote_entropy_predictions(guid_mapped_entropy& adjusted_entropy, const entity_id& predictable_entity, const cosmos& predicted_cosmos) {
	entity_intent release_intent;
	release_intent.pressed_flag = false;

	for (auto e : predicted_cosmos.get(processing_subjects::WITH_PAST_CONTAGIOUS)) {
		const bool is_locally_controlled_entity = e == predictable_entity;
		
		if (is_locally_controlled_entity)
			continue;

		for (const auto g : e.guns_wielded()) {
			if (g.get<components::gun>().trigger_pressed) {
				if (g.get_current_slot().raw_id.type == slot_function::PRIMARY_HAND) {
					release_intent.intent = intent_type::CROSSHAIR_PRIMARY_ACTION;
				}
				else {
					release_intent.intent = intent_type::CROSSHAIR_SECONDARY_ACTION;
				}

				adjusted_entropy.entropy_per_entity[e.get_guid()].push_back(release_intent);
			}
		}
	}
}

simulation_receiver::unpacking_result simulation_receiver::unpack_deterministic_steps(cosmos& referential_cosmos, cosmos& last_delta_unpacked) {
	unpacking_result result;
	auto& reconciliate_predicted = result.reconciliate_predicted;
	auto& entropies_to_simulate = result.entropies_to_simulate;

	auto new_commands = jitter_buffer.buffer;
	jitter_buffer.buffer.clear();

	for (size_t i = 0; i < new_commands.size(); ++i) {
		auto& new_command = new_commands[i];

		if (new_command.step_type == step_packaged_for_network::type::NEW_ENTROPY_WITH_HEARTBEAT) {
			cosmic_delta::decode(last_delta_unpacked, new_command.delta);
			referential_cosmos = last_delta_unpacked;

			entropies_to_simulate.clear();
			reconciliate_predicted = true;
		}
		else
			ensure(new_command.step_type == step_packaged_for_network::type::NEW_ENTROPY);

		step_to_simulate sim;
		sim.resubstantiate = new_command.shall_resubstantiate;
		sim.entropy = new_command.entropy;

		entropies_to_simulate.emplace_back(sim);

		const auto& actual_server_step = sim.entropy;
		const auto& predicted_server_step = predicted_steps.front();

		if (sim.resubstantiate || actual_server_step != predicted_server_step) {
			reconciliate_predicted = true;
		}

		if (new_command.next_client_commands_accepted) {
			ensure(predicted_steps.size() > 0);
			predicted_steps.erase(predicted_steps.begin());
		}
	}

	return std::move(result);
}

simulation_receiver::misprediction_candidate_entry simulation_receiver::acquire_potential_misprediction(const const_entity_handle& e) const {
	misprediction_candidate_entry candidate;
	
	if(e.has_logic_transform())
		candidate.transform = e.logic_transform();

	candidate.id = e.get_id();

	return candidate;
}

std::vector<simulation_receiver::misprediction_candidate_entry> simulation_receiver::acquire_potential_mispredictions(const cosmos& predicted_cosmos_before_reconciliation) const {
	const auto& cosmos = predicted_cosmos_before_reconciliation;

	const auto& unpredictables_with_past_contagious = cosmos.get(processing_subjects::WITH_PAST_CONTAGIOUS);
	const auto& unpredictables_infected = cosmos.systems_insignificant.get<past_infection_system>().infected_entities;

	std::vector<misprediction_candidate_entry> potential_mispredictions;
	potential_mispredictions.reserve(unpredictables_with_past_contagious.size() + unpredictables_infected.size());

	for (const auto& e : unpredictables_with_past_contagious) {
		potential_mispredictions.push_back(acquire_potential_misprediction(e));
	}

	for (const auto& e : unpredictables_infected) {
		if (cosmos[e].alive()) {
			potential_mispredictions.push_back(acquire_potential_misprediction(cosmos[e]));
		}
	}

	return std::move(potential_mispredictions);
}

void simulation_receiver::drag_mispredictions_into_past(const cosmos& predicted_cosmos, const std::vector<misprediction_candidate_entry>& mispredictions) const {
	for (const auto e : mispredictions) {
		const const_entity_handle reconciliated_entity = predicted_cosmos[e.id];
		
		const bool identity_matches = reconciliated_entity.alive() && reconciliated_entity.has_logic_transform();

		if (!identity_matches)
			continue;

		const auto& reconciliated_transform = reconciliated_entity.logic_transform();
		const bool is_contagious_agent = reconciliated_entity.get_flag(entity_flag::IS_PAST_CONTAGIOUS);
		const bool should_smooth_rotation = !is_contagious_agent || predicted_cosmos[reconciliated_entity.get<components::driver>().owned_vehicle].alive();

		auto& interp_data = predicted_cosmos.systems_insignificant.get<interpolation_system>().get_data(reconciliated_entity);

		const bool shouldnt_smooth = reconciliated_entity.has<components::crosshair>();
		bool misprediction_detected = false;

		if (!shouldnt_smooth && (reconciliated_transform.pos - e.transform.pos).length_sq() > 1.f) {
			interp_data.positional_slowdown_multiplier = static_cast<float>(predicted_steps.size());
			misprediction_detected = true;
		}

		if (should_smooth_rotation && std::abs(reconciliated_transform.rotation - e.transform.rotation) > 1.f) {
			interp_data.rotational_slowdown_multiplier = static_cast<float>(predicted_steps.size());
			misprediction_detected = true;
		}

		if (identity_matches || (!misprediction_detected && !is_contagious_agent)) {
			predicted_cosmos.systems_insignificant.get<past_infection_system>().uninfect(reconciliated_entity);
		}
	}
}
