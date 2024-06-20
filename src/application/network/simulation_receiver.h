#pragma once
#include <unordered_set>

#include "augs/log.h"

#include "augs/network/jitter_buffer.h"
#include "augs/templates/logically_empty.h"
#include "game/cosmos/cosmic_functions.h"

#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/past_infection_system.h"

#include "application/network/server_step_entropy.h"
#include "application/network/simulation_receiver_settings.h"

#include "application/network/interpolation_transfer.h"
#include "application/arena/synced_dynamic_vars.h"

#define USE_CLIENT_PREDICTION 1
#define TEST_DESYNC_DETECTION 0

struct misprediction_candidate_entry {
	entity_id id;
	components::transform transform;
};

struct steps_unpacking_result {
	bool should_repredict = false;
	bool malicious_server = false;
	bool desync = false;
	std::size_t total_accepted = static_cast<std::size_t>(-1);
};

class simulation_receiver {
public:
	using received_entropy_type = compact_server_step_entropy;
	using simulated_entropy_type = server_step_entropy;
private:

	template <class H>
	misprediction_candidate_entry acquire_potential_misprediction(const H& e) const {
		misprediction_candidate_entry candidate;
		
		if (const auto t = e.find_logic_transform()) {
			candidate.transform = *t;
		}

		candidate.id = e.get_id();

		return candidate;
	}
	
	std::vector<misprediction_candidate_entry> acquire_potential_mispredictions(
		const std::unordered_set<entity_id>&, 
		const cosmos& predicted_cosmos_before_reconciliation
	) const;

	void drag_mispredictions_into_past(
		const simulation_receiver_settings&,
		interpolation_system&,
		past_infection_system&, 
		const cosmos& predicted_arena, 
		const std::vector<misprediction_candidate_entry>& mispredictions
	) const;

	void predict_intents_of_remote_entities(
		simulated_entropy_type& adjusted_entropy, 
		const entity_id locally_controlled_entity, 
		const cosmos& predicted_arena
	);

public:

	struct incoming_entropy_entry {
		server_step_entropy_meta meta;
		received_entropy_type payload;

		std::optional<synced_dynamic_vars> new_dynamic_vars;
	};

	std::optional<synced_dynamic_vars> next_dynamic_vars;

	std::vector<prestep_client_context> incoming_contexts;
	std::vector<incoming_entropy_entry> incoming_entropies;
	std::vector<simulated_entropy_type> predicted_entropies;
	interpolation_transfer_caches transfer_caches;

	bool schedule_reprediction = false;

	void clear_incoming() {
		incoming_contexts.clear();
		incoming_entropies.clear();
	}

	void clear() {
		clear_incoming();
		predicted_entropies.clear();
	}

	void acquire_next_dynamic_vars(
		const synced_dynamic_vars& vars
	) {
		next_dynamic_vars = vars;
	}

	void acquire_next_server_entropy(
		const prestep_client_context& context,
		const server_step_entropy_meta& meta,
		const received_entropy_type& payload
	) {
		// LOG_NVPS(context.num_entropies_accepted);
		incoming_contexts.push_back(context);
		incoming_entropies.push_back({meta, payload, next_dynamic_vars});

		next_dynamic_vars = std::nullopt;

		if (incoming_contexts.size() < incoming_entropies.size()) {
			incoming_contexts.emplace_back();

			ensure_eq(incoming_contexts.size(), incoming_entropies.size());
		}
	}

	template <class F, class A, class S1, class S2>
	steps_unpacking_result unpack_deterministic_steps(
		const simulation_receiver_settings& settings,

		interpolation_system& interp, 
		past_infection_system& past,

		const entity_id locally_controlled_entity, 

		synced_dynamic_vars& sv_dynamic_vars,

		F&& unpack_entropy,

		A& referential_arena, 
		A& predicted_arena, 

		S1 advance_referential,
		S2 advance_predicted
	) {
		steps_unpacking_result result;

		auto& contexts = incoming_contexts;
		auto& entropies = incoming_entropies;

		ensure_geq(contexts.size(), entropies.size());

		if (contexts.size() - entropies.size() >= 2) {
			/* Only one context can be ahead of the proper entropies. */
			result.malicious_server = true;
			return result;
		}

		auto& repredict = result.should_repredict;

		if (schedule_reprediction) {
			if (!predicted_entropies.empty()) {
				repredict = true;
				schedule_reprediction = false;
			}
		}

		{
			auto num_total_accepted_entropies = static_cast<std::size_t>(0);

			for (std::size_t i = 0; i < entropies.size(); ++i) {
				/* If a new player was added, always reinfer. */
				const auto& actual_server_step = entropies[i];

				const auto& referential_cosmos = referential_arena.get_cosmos();

				const auto& meta = actual_server_step.meta;

				{
					const auto received_hash = meta.state_hash;

					if (received_hash.has_value()) {
#if TEST_DESYNC_DETECTION
						auto total = referential_cosmos.get_total_steps_passed();
						bool simulate_desync = false;

						for (auto i = total; i < total + 1; ++i) {
							if (i % (128 * 6) == 0) {
								simulate_desync = true;
							}
						}

						if (simulate_desync) {
							const auto it = referential_arena.get_cosmos()[locally_controlled_entity];

							if (it) {
								LOG("Altering the state for a test.");
								auto& s = it.template get<components::sentience>();
								s.template get<health_meter_instance>().maximum += 1.f;
							}
							else {
								LOG("Looks like controlled entity is dead!");
							}
						}
#endif

						const auto client_state_hash = 
							referential_cosmos.template calculate_solvable_signi_hash<uint32_t>()
						;

						const auto step_number = referential_cosmos.get_total_steps_passed();

						if (*received_hash != client_state_hash) {
							LOG(
								"Client desynchronized at step: %x. Hashes differ.\nExpected: %x\nActual: %x\n",
								step_number,
							   	*received_hash,
							   	client_state_hash
							);

							result.desync = true;
						}
					}
				}

				{
					const bool shall_reinfer = meta.reinference_necessary || logically_set(actual_server_step.payload.general.added_player);

					if (shall_reinfer) {
						LOG("Added player in the next entropy. Will reinfer to sync.");
						cosmic::reinfer_solvable(referential_arena.get_cosmos());
					}

					{
						const auto& actual_server_entropy = unpack_entropy(actual_server_step.payload);

						{
							const auto& new_dynamic_vars = actual_server_step.new_dynamic_vars;

							if (new_dynamic_vars.has_value()) {
								sv_dynamic_vars = *new_dynamic_vars;

								LOG(
									"New synced_dynamic_vars. Step: %x, Run ranked logic: %x, FF: %x; preassigned teams: %x %x", 
									referential_arena.get_cosmos().get_total_steps_passed(),
									sv_dynamic_vars.is_ranked_server(),
									sv_dynamic_vars.friendly_fire,
									sv_dynamic_vars.preassigned_factions,
									sv_dynamic_vars.all_assigned_present
								);
							}
						}

						advance_referential(actual_server_entropy);

						const bool already_found_reason_to_repredict = repredict;

						if (!already_found_reason_to_repredict) {
							if (num_total_accepted_entropies < predicted_entropies.size())
							{
								const auto& predicted_server_entropy = predicted_entropies[num_total_accepted_entropies];

								if (shall_reinfer || !(actual_server_entropy == predicted_server_entropy)) {
									repredict = true;
								}
							}
							else
							{
								LOG("The client has fallen back behind the server. Repredicting the world just in case.");
								repredict = true;
							}
						}
					}
				}

				{
					const auto num_accepted = contexts[i].num_entropies_accepted;

					if (num_accepted != 1) {
						/* We'll need to nudge into the future or into the past. */

						repredict = true;
					}

					num_total_accepted_entropies += num_accepted;
				}
			}

			erase_first_n(incoming_contexts, entropies.size());
			entropies.clear();

			result.total_accepted = num_total_accepted_entropies;

			auto& predicted = predicted_entropies;

			// LOG("TA: %x", num_total_accepted_entropies);

			if (num_total_accepted_entropies <= predicted.size()) {
				erase_first_n(predicted, num_total_accepted_entropies);
			}
			else {
				LOG_NVPS(num_total_accepted_entropies, predicted.size());
				result.malicious_server = true;
				return result;
			}
		}

#if USE_CLIENT_PREDICTION
		if (repredict) {
			auto& predicted_cosmos = predicted_arena.get_cosmos();

			const auto potential_mispredictions = acquire_potential_mispredictions(
				past.infected_entities, 
				predicted_cosmos
			);

			::save_interpolations(transfer_caches, std::as_const(predicted_cosmos));

			predicted_arena.transfer_all_solvables(referential_arena);

			for (auto& predicted_step_entropy : predicted_entropies) {
				predict_intents_of_remote_entities(
					predicted_step_entropy,
					locally_controlled_entity, 
					predicted_cosmos
				);

				advance_predicted(predicted_step_entropy);
			}

			::restore_interpolations(transfer_caches, predicted_cosmos);

			drag_mispredictions_into_past(
				settings, 
				interp, 
				past, 
				predicted_cosmos, 
				potential_mispredictions
			);
		}
#else
		(void)settings;
		(void)interp;
		(void)past;
		(void)predicted_arena;
		(void)locally_controlled_entity;
		(void)advance_predicted;
#endif

		return result;
	}
};