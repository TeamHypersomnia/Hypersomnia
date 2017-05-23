#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "augs/network/network_server.h"
#include "augs/misc/jitter_buffer.h"

#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"

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
		bool next_commands_accepted = false;

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

	struct server_meta {
		struct controlled_character {
			controlled_character(entity_id id = entity_id()) : id(id) {}

			entity_id id;
			bool occupied = false;
		};

		std::vector<controlled_character> characters;

		entity_id assign_new_character() {
			for (auto& c : characters) {
				if (!c.occupied) {
					c.occupied = true;
					return c.id;
				}
			}

			ensure(false);
			return entity_id();
		}

		void free_character(const entity_id id) {
			for (auto& c : characters) {
				if (c.id == id) {
					ensure(c.occupied);
					c.occupied = false;
					return;
				}
			}

			ensure(false);
		}
	} scene;

	endpoint& get_endpoint(const augs::network::endpoint_address);
	void disconnect(const augs::network::endpoint_address, const bool gracefully = false);
	void deinit_endpoint(const augs::network::endpoint_address, const bool gracefully = false);
	void deinit_endpoint(endpoint&, const bool gracefully = false);
public:
	void wait_for_listen_server();

	void process(const config_lua_table& cfg, game_window&, bool start_alternative_server = false);
};