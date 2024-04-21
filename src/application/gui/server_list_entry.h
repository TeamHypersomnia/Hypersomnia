#pragma once
#include "augs/network/network_types.h"
#include "application/masterserver/server_heartbeat.h"
#include "application/masterserver/masterserver_client.h"

enum class server_entry_state {
	GIVEN_UP,
	AWAITING_RESPONSE,
	PING_MEASURED
};

namespace augs {
	double steady_secs();
}

struct ping_progress {
	uint32_t ping = -1;
	uint64_t ping_sequence = -1;

	net_time_t when_sent_first_ping = -1;
	net_time_t when_sent_last_ping = -1;

	server_entry_state state = server_entry_state::AWAITING_RESPONSE;

	bool found_on_internal_network = false;

	bool responding() const {
		return state == server_entry_state::PING_MEASURED;
	}

	rgba get_ping_color() const {
		if (state == server_entry_state::PING_MEASURED) {
			return white;
		}

		return rgba(255/2, 255/2, 255/2, 255);
	}

	auto get_ping_string() const {
		if (state == server_entry_state::PING_MEASURED) {
			return std::to_string(ping);
		}

		if (state == server_entry_state::AWAITING_RESPONSE) {
			const auto current_time = augs::steady_secs();

			const auto num_dots = uint64_t(current_time * 3) % 3 + 1;

			return std::string(num_dots, '.');
		}

		return std::string("?");
	}

	void set_ping_from(const net_time_t current_time) {
		ping = static_cast<int>((current_time - when_sent_last_ping) * 1000);
	}
};

struct server_list_entry {
	netcode_address_t address;
	webrtc_id_type webrtc_id;
	masterserver_entry_meta meta;

	server_heartbeat heartbeat;

	ping_progress progress;

	bool is_community_server() const {
		return !meta.is_official;
	}

	bool is_official_server() const {
		return meta.is_official;
	}

	std::string get_connect_string() const;
	netcode_address_t get_connect_address() const;

	bool is_set() const;
	bool is_behind_nat() const;
};
