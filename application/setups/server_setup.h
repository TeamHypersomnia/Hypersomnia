#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

#include "augs/network/network_server.h"
#include "augs/misc/jitter_buffer.h"

#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"

#include "game/scene_managers/networked_testbed.h"

#include "setup_base.h"

class game_window;

class server_setup : public setup_base {
	std::mutex mtx;
	std::condition_variable cv;

	volatile bool server_ready = false;

	augs::network::server serv;
	augs::network::server alternative_serv;
	augs::network::server& choose_server(augs::network::endpoint_address addr);

	struct endpoint {
		augs::network::endpoint_address addr;
		//std::vector<guid_mapped_entropy> commands;
		augs::jitter_buffer<guid_mapped_entropy> commands;
		step_packaged_for_network next_command;
		entity_id controlled_entity;
		bool sent_welcome_message = false;
		std::string nickname;

		std::string nick_and_ip() const;

		bool operator==(augs::network::endpoint_address b) const {
			return addr == b;
		}
	};

	cosmos hypersomnia = cosmos(3000);

	std::vector<endpoint> endpoints;
	scene_managers::networked_testbed_server scene;

	endpoint& get_endpoint(const augs::network::endpoint_address);
	void disconnect(const augs::network::endpoint_address, const bool gracefully = false);
	void deinit_endpoint(const augs::network::endpoint_address, const bool gracefully = false);
	void deinit_endpoint(endpoint&, const bool gracefully = false);
public:
	void wait_for_listen_server();

	void process(game_window&, bool start_alternative_server = false);
};