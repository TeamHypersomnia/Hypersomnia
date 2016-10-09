#include "augs/templates.h"
#include "simulation_receiver.h"
#include "game/transcendental/cosmic_delta.h"
#include "augs/network/network_client.h"

#include "game/components/driver_component.h"
#include "game/components/flags_component.h"
#include "game/components/crosshair_component.h"

#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmos.h"

void simulation_receiver::acquire_next_packaged_step(const step_packaged_for_network& step) {
	jitter_buffer.acquire_new_command(step);
}

simulation_receiver::mismatch_candidate_entry simulation_receiver::acquire_potential_mismatch(const const_entity_handle& e) const {
	mismatch_candidate_entry candidate;
	
	if(e.has_logic_transform())
		candidate.transform = e.logic_transform();

	candidate.id = e.get_id();

	return candidate;
}

std::vector<simulation_receiver::mismatch_candidate_entry> simulation_receiver::acquire_potential_mismatches(const cosmos& predicted_cosmos_before_reconciliation) const {
	const auto& cosmos = predicted_cosmos_before_reconciliation;

	const auto& unpredictables_with_past_contagious = cosmos.get(processing_subjects::WITH_PAST_CONTAGIOUS);
	const auto& unpredictables_infected = cosmos.systems_insignificant.get<past_infection_system>().infected_entities;

	std::vector<mismatch_candidate_entry> potential_mismatches;
	potential_mismatches.reserve(unpredictables_with_past_contagious.size() + unpredictables_infected.size());

	for (const auto& e : unpredictables_with_past_contagious) {
		potential_mismatches.push_back(acquire_potential_mismatch(e));
	}

	for (const auto& e : unpredictables_infected) {
		if (cosmos[e].alive()) {
			potential_mismatches.push_back(acquire_potential_mismatch(cosmos[e]));
		}
	}

	return potential_mismatches;
}

void simulation_receiver::drag_mismatches_into_past(const cosmos& predicted_cosmos, const std::vector<mismatch_candidate_entry>& mismatches) const {
	for (const auto e : mismatches) {
		const const_entity_handle reconciliated_entity = predicted_cosmos[e.id];
		
		const bool identity_matches = reconciliated_entity.alive() && reconciliated_entity.has_logic_transform();

		if (!identity_matches)
			continue;

		const auto& reconciliated_transform = reconciliated_entity.logic_transform();
		const bool is_contagious_agent = reconciliated_entity.get_flag(entity_flag::IS_PAST_CONTAGIOUS);
		const bool should_smooth_rotation = !is_contagious_agent || predicted_cosmos[reconciliated_entity.get<components::driver>().owned_vehicle].alive();

		auto& interp_data = predicted_cosmos.systems_insignificant.get<interpolation_system>().get_data(reconciliated_entity);

		const bool shouldnt_smooth = reconciliated_entity.has<components::crosshair>();
		bool mismatch_detected = false;

		if (!shouldnt_smooth && (reconciliated_transform.pos - e.transform.pos).length_sq() > 1.f) {
			interp_data.positional_slowdown_multiplier = predicted_steps.size();// std::max(10u, predicted_steps.size());
			mismatch_detected = true;
			//.set_slowdown_multiplier(reconciliated_entity, static_cast<float>(predicted_steps.size()));
		}

		if (!is_contagious_agent && std::abs(reconciliated_transform.rotation - e.transform.rotation) > 1.f) {
			interp_data.rotational_slowdown_multiplier = predicted_steps.size();
			mismatch_detected = true;
		}

		if (identity_matches || (!mismatch_detected && !is_contagious_agent)) {
			predicted_cosmos.systems_insignificant.get<past_infection_system>().uninfect(reconciliated_entity);
		}
	}
}
