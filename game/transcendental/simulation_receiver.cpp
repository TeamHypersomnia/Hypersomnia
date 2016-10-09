#include "augs/templates.h"
#include "simulation_receiver.h"
#include "game/transcendental/cosmic_delta.h"
#include "augs/network/network_client.h"

#include "game/components/past_contagious_component.h"

#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmos.h"

void simulation_receiver::acquire_next_packaged_step(const step_packaged_for_network& step) {
	jitter_buffer.acquire_new_command(step);
}

void simulation_receiver::drag_mismatches_into_past(cosmos& predicted_cosmos, const std::vector<mismatch_candidate_entry>& mismatches) {
	for (const auto& e : mismatches) {
		const const_entity_handle& reconciliated_entity = predicted_cosmos[e.id];
		const auto& reconciliated_transform = reconciliated_entity.logic_transform();
		const bool is_contagious_agent = reconciliated_entity.has<components::past_contagious>();

		auto& interp_data = predicted_cosmos.systems_insignificant.get<interpolation_system>().get_data(reconciliated_entity);

		if ((reconciliated_transform.pos - e.transform.pos).length_sq() > 1.f) {
			interp_data.positional_slowdown_multiplier = predicted_steps.size();// std::max(10u, predicted_steps.size());
			//.set_slowdown_multiplier(reconciliated_entity, static_cast<float>(predicted_steps.size()));
		}

		if (!is_contagious_agent && std::abs(reconciliated_transform.rotation - e.transform.rotation) > 1.f) {
			interp_data.rotational_slowdown_multiplier = predicted_steps.size();
		}
	}
}
