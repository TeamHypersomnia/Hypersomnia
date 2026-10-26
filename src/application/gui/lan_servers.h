#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
#include "augs/log.h"
#include "augs/templates/container_templates.h"
#include "augs/network/network_types.h"
#include "application/gui/server_list_entry.h"
#include "application/masterserver/heartbeat_on_ping.h"

#if BUILD_NATIVE_SOCKETS
#include "augs/network/netcode_sockets.h"
#include "application/masterserver/masterserver_requests.h"
#include "application/network/address_utils.h"
#endif

/*
	The servers on the local network the server list does not have (see browse_servers_gui.cpp) - found by a broadcast
	to the first game ports at every Refresh list or Refresh ping (and the first look), then at the ports of the instances
	each one tells in its heartbeat (see heartbeat_on_ping.h).
*/

/*
	A heartbeat to a ping taken into its entry - a favorite the list does not have, or one found on the local network.
*/

inline void take_heartbeat_on_ping(server_list_entry& s, server_heartbeat&& heartbeat) {
	/*
		A favorite stays the one it was starred as (its label) - else it would not be found again
		(see update_favorite_only_list).
	*/

	const auto label = s.heartbeat.instance_label;

	s.heartbeat = std::move(heartbeat);
	s.heartbeat_from_ping = true;

	if (s.favorite_only) {
		s.heartbeat.instance_label = label;
	}

	/*
		Found on the local network: joined right at the address it answered from.
	*/

	if (s.lan) {
		s.heartbeat.nat = nat_detection_result();
		s.heartbeat.internal_network_address = std::nullopt;
	}
}

struct lan_servers {
	std::vector<server_list_entry> list;

	bool discovery_pending = false;
	uint64_t discovery_sequence = -1;
	net_time_t discovery_sent_at = 0;

	bool any_responding() const {
		return std::any_of(list.begin(), list.end(), [](const server_list_entry& s) {
			return s.progress.responding();
		});
	}

	/*
		One known before stays while asked anew (Refresh ping) - only one that does not answer goes.
	*/

	static bool shown(const server_list_entry& s) {
		const bool asked_anew = s.progress.state == server_entry_state::AWAITING_RESPONSE && s.heartbeat_from_ping;
		return s.progress.responding() || asked_anew;
	}

	/*
		The instances launched together (a dedicated server's, a tournament's matches) as one of them tells: the ones
		no longer there go at once, the new ones are added - pinged once each by the ping loop, shown once they answer.
		Instances launched apart (e.g. two games hosting on one computer) tell other first ports - never touched here.
	*/

	template <class F>
	void take_ports(const netcode_address_t& teller, const port_type first_port, const uint16_t told_num_ports, F&& on_server_list) {
		constexpr uint16_t max_lan_ports_v = 64;

		const auto num_ports = std::min(told_num_ports, max_lan_ports_v);

		if (first_port == 0 || num_ports == 0) {
			return;
		}

		auto same_host = [&](const netcode_address_t& a) {
			auto a_host = a;
			auto b_host = teller;
			a_host.port = 0;
			b_host.port = 0;
			return a_host == b_host;
		};

		const auto end_port = static_cast<uint32_t>(first_port) + num_ports;

		if (num_ports > 1) {
			LOG("%x on the local network has %x ports: %x-%x.", ::ToString(teller), num_ports, first_port, end_port - 1);
		}

		auto in_range = [&](const port_type port) {
			return port >= first_port && static_cast<uint32_t>(port) < end_port;
		};

		erase_if(list, [&](const server_list_entry& s) {
			return same_host(s.address) && s.lan_first_port == first_port && !in_range(s.address.port);
		});

		for (auto port = static_cast<uint32_t>(first_port); port < end_port; ++port) {
			auto address = teller;
			address.port = static_cast<port_type>(port);

			const bool known = std::any_of(list.begin(), list.end(), [&](const server_list_entry& s) {
				return s.address == address;
			});

			if (known || on_server_list(address)) {
				continue;
			}

			auto entry = server_list_entry();
			entry.lan = true;
			entry.address = address;
			entry.lan_first_port = first_port;
			entry.lan_num_ports = num_ports;
			entry.heartbeat.server_name = ::ToString(address);
			entry.progress.found_on_internal_network = true;

			list.emplace_back(std::move(entry));
		}
	}

	/*
		A heartbeat to a ping from an entry found here: its ports are taken too.
	*/

	template <class F>
	void take_heartbeat(server_list_entry& s, heartbeat_on_ping&& received, F&& on_server_list) {
		::take_heartbeat_on_ping(s, std::move(received.heartbeat));

		const auto teller = s.address;
		s.lan_first_port = received.header.lan_first_port;
		s.lan_num_ports = received.header.lan_num_ports;

		/*
			The list changes inside - nothing of s is used after.
		*/

		take_ports(teller, received.header.lan_first_port, received.header.lan_num_ports, std::forward<F>(on_server_list));
	}

#if BUILD_NATIVE_SOCKETS
	bool broadcast_enabled = false;
	std::vector<netcode_address_t> broadcast_addresses;
	net_time_t when_got_broadcast_addresses = -1;

	template <class F>
	void on_found(netcode_socket_t& socket, const netcode_address_t& from, const net_time_t now, F&& on_server_list) {
		if (on_server_list(from)) {
			return;
		}

		for (const auto& s : list) {
			if (s.address == from) {
				return;
			}
		}

		LOG("Found a server on the local network at: %x", ::ToString(from));

		auto entry = server_list_entry();
		entry.lan = true;
		entry.address = from;
		entry.lan_first_port = from.port;
		entry.lan_num_ports = 1;
		entry.heartbeat.server_name = ::ToString(from);

		auto& p = entry.progress;
		p.found_on_internal_network = true;
		p.ping_sequence = discovery_sequence;
		p.when_sent_first_ping = discovery_sent_at;
		p.when_sent_last_ping = discovery_sent_at;
		p.set_ping_from(now);
		p.state = server_entry_state::PING_MEASURED;

		list.emplace_back(std::move(entry));

		::request_heartbeat_on_ping(socket, from, discovery_sequence);
	}

	/*
		also_these: the favorites on the local network, asked directly too - a network that blocks broadcast (a LAN
		party's router) still shows them here, with all their ports (a tournament's matches), as if broadcast found them.
	*/

	void discover(netcode_socket_t& socket, const uint64_t sequence, const net_time_t now, const std::vector<netcode_address_t>& also_these) {
		if (!broadcast_enabled) {
			broadcast_enabled = ::enable_broadcast(socket);

			if (!broadcast_enabled) {
				LOG("Could not enable broadcast on the server browser's socket - only the favorites on the local network are asked.");
			}
		}

		/*
			The interfaces asked anew only now and then - on some systems asking takes a while.
		*/

		if (when_got_broadcast_addresses < 0 || now - when_got_broadcast_addresses > 30.0) {
			broadcast_addresses = ::get_broadcast_addresses();
			when_got_broadcast_addresses = now;
		}

		discovery_sequence = sequence;
		discovery_sent_at = now;

		/*
			Asked anew: the ones not answering within the ping loop's time go - the rest stay where they are.
		*/

		erase_if(list, [](const server_list_entry& s) {
			return s.progress.state == server_entry_state::GIVEN_UP;
		});

		for (auto& s : list) {
			s.progress = {};
			s.progress.found_on_internal_network = true;
		}

		auto send = [&](const netcode_address_t& target) {
			auto packet = ::make_ping_packet(target, discovery_sequence);
			netcode_socket_send_packet(&socket, &packet.to, packet.bytes.data(), packet.bytes.size());
		};

		if (broadcast_enabled) {
			for (const auto& broadcast : broadcast_addresses) {
				for (port_type i = 0; i < NUM_LAN_GAME_PORTS_V; ++i) {
					auto target = broadcast;
					target.port = static_cast<port_type>(DEFAULT_GAME_PORT_V + i);
					send(target);
				}
			}

			LOG("Broadcast to %x addresses for the servers on the local network.", broadcast_addresses.size());
		}

		for (const auto& favorite : also_these) {
			send(favorite);
		}

		if (!also_these.empty()) {
			LOG("Asked %x favorites on the local network directly.", also_these.size());
		}
	}

	bool is_discovery_answer(const uint64_t sequence) const {
		return discovery_sent_at > 0 && sequence == discovery_sequence;
	}
#endif
};
