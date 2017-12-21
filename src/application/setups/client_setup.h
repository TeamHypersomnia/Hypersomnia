#pragma once
#include "setup_base.h"

#include "game/transcendental/cosmos.h"

#include "view/network/simulation_receiver.h"

#include "augs/network/network_client.h"

class client_setup : public setup_base {
public:
	bool predict_entropy = true;

	all_logical_assets logical_assets;

	cosmos cosm = cosmos(3000);
	cosmos initial_cosm = cosmos(3000);

	cosmos cosm_last_snapshot = cosmos(3000);
	cosmos extrapolated_cosm = cosmos(3000);

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	entity_id currently_controlled_character;

	bool last_stepped_was_extrapolated = false;
	bool complete_state_received = false;

	bool detailed_step_log = false;

	augs::network::client client;
	simulation_receiver receiver;

	void process(
		augs::window&,
		viewing_session&
	);

	void init(
		augs::window&, 
		viewing_session&,
		const std::string recording_filename = "recorded.inputs",
		const bool use_alternative_port = false
	);

	void process_once(
		augs::window&,
		viewing_session&,
		const augs::local_entropy& precollected, 
		const bool swap_buffers = true
	);
};
