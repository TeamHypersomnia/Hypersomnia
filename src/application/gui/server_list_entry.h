#pragma once
#include <cstdint>
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
	masterserver_entry_meta meta;

	server_heartbeat heartbeat;

	ping_progress progress;

	bool unlisted = false;

	/*
		A favorite server not on the server list - only its key, name and the address it was last at
		(pinged there, joined there expecting the key).
	*/

	bool favorite_only = false;
	std::string favorite_address;

	/*
		The server list has another server (another key) at the favorite's address - named here; the favorite is
		not pinged there (that one would answer).
	*/

	std::string other_server_at_address;

	bool is_community_server() const {
		return meta.is_community_server();
	}

	bool is_official_server() const {
		return meta.is_official_server();
	}

	/*
		The key from the server list, if the masterserver gave one - what the client expects in the handshake.
	*/

	/*
		An official server's time zone for happy hours - see masterserver_entry_meta.
	*/

	std::string get_location_id() const {
		return std::string(meta.location_id);
	}

	std::string get_instance_label() const {
		return std::string(heartbeat.instance_label);
	}

	std::optional<std::array<uint8_t, 32>> get_public_key() const {
		for (const auto b : meta.server_public_key) {
			if (b != 0) {
				return meta.server_public_key;
			}
		}

		return std::nullopt;
	}

	std::string get_my_connect_string() const;

	/*
		The same server: by its key if it has one, else by the address.
	*/

	bool same_server(const server_list_entry& b) const {
		if (const auto key = get_public_key()) {
			return key == b.get_public_key();
		}

		return !b.get_public_key().has_value() && address == b.address;
	}

	/*
		What a join from the server list names: where it is and its key (see connect_target.h).
	*/

	std::string get_join_string() const;

	/*
		Where a friend joins it (invites, the Steam join link): its public IP, rtc: if behind NAT, a web server's
		WebRTC id - never a domain (only a label the server declares) nor an address in this player's network.
	*/

	std::string get_invite_address() const;

	std::string get_web_connect_string() const;
	std::string get_native_connect_string(bool for_sending) const;

	/*
		"domain:port" if the server reports a domain, otherwise empty.
		The domain is only a label the server declares for itself - official or not - never a proof of anything.
	*/

	std::string get_domain_with_port() const;

	netcode_address_t get_ip_address(bool for_sending) const;

	bool is_set() const;
	bool is_behind_nat() const;
	bool is_internal_network() const;
};
