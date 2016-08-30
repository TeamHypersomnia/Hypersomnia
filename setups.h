#pragma once
#include <mutex>
#include "augs/window_framework/window.h"
#include "augs/misc/machine_entropy.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/simulation_receiver.h"
#include "game/scene_managers/networked_testbed.h"
#include "augs/network/network_client.h"
#include "game/transcendental/step_and_entropy_unpacker.h"

class game_window;

class setup_base {
public:
	augs::window::event::keys::key exit_key = augs::window::event::keys::ESC;
	volatile bool should_quit = false;

	bool process_exit_key(const augs::machine_entropy::local_type&);
};

class local_setup : public setup_base {
public:
	void process(game_window&);
};

class server_setup : public setup_base {
	std::mutex mtx;
	std::condition_variable cv;

	volatile bool server_ready = false;
public:
	void wait_for_listen_server();

	void process(game_window&, bool start_alternative_server = false);
};

class client_setup : public setup_base {
public:
	cosmos hypersomnia = cosmos(3000);
	cosmos initial_hypersomnia = cosmos(3000);

	cosmos hypersomnia_last_snapshot = cosmos(3000);
	cosmos extrapolated_hypersomnia = cosmos(3000);
	
	viewing_session session;

	step_and_entropy_unpacker input_unpacker;
	scene_managers::networked_testbed_client scene;

	bool last_stepped_was_extrapolated = false;
	bool complete_state_received = false;

	bool detailed_step_log = false;

	augs::network::client client;
	simulation_receiver receiver;

	void process(game_window&);

	void init(game_window&, const std::string recording_filename = "recorded.inputs", const bool use_alternative_port = false);
	void process_once(game_window&, const augs::machine_entropy::local_type& precollected, const bool swap_buffers = true);
};

class two_clients_and_server_setup : public setup_base {
public:
	void process(game_window&);
};

class determinism_test_setup : public setup_base {
public:
	void process(game_window&);
};