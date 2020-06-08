#pragma once
#include "augs/log.h"
#include "augs/network/netcode_sockets.h"
#include "application/nat/nat_type.h"
#include "application/nat/nat_traversal_settings.h"
#include "application/nat/nat_detection_settings.h"
#include "application/nat/netcode_packet_queue.h"
#include "application/nat/stun_session.h"
#include "augs/network/port_type.h"
#include "augs/misc/randomization.h"
#include "application/masterserver/nat_traversal_step_payload.h"

struct stun_server_provider;

struct nat_traversal_input {
	netcode_address_t masterserver_address;
	netcode_address_t traversed_address;

	nat_detection_result client;
	nat_detection_result server;
	nat_detection_settings detection_settings;
	nat_traversal_settings traversal_settings;
};

class nat_traversal_session {
public:
	enum class state {
		INIT,

		REQUESTING_REMOTE_PORT_INFO,
		SERVER_STUN_REQUIREMENT_MET,

		TRAVERSING,
		TRAVERSAL_COMPLETE,

		TIMED_OUT
	};

	const nat_traversal_input input;

private:
	randomization stun_rng = randomization::from_random_device();
	const nat_session_guid_type session_guid;

	std::optional<stun_session> stun_in_progress;
	state traversal_state = state::INIT;

	net_time_t when_last_masterserver_request = -1;
	net_time_t when_began = -1;

	std::string full_log;
	port_type chosen_masterserver_port_probe = 0;
	port_type last_server_stunned_port = 0;

	port_type opened_remote_port = 0;
	int requests_fired = 0;

	netcode_packet_queue packet_queue;
	bool client_holes_opened = false;

	template <class... Args>
	void log_info(Args&&... args) {
		const auto s = typesafe_sprintf(std::forward<Args>(args)...);

		full_log += s + "\n";
		LOG(s);
	}

	bool timed_out();
	void advance_init();

	void handle_packet(const netcode_address_t& from, uint8_t* buffer, const int bytes_received);
	void receive_packets(netcode_socket_t socket);

	void set(state);

	netcode_address_t get_current_masterserver_address() const;

public:
	nat_traversal_session(const nat_traversal_input&, stun_server_provider& stun_provider); 

	void advance(const netcode_socket_t&);
	state get_current_state() const;
	const std::string& get_full_log() const;

	port_type get_opened_port_at_target_host() const;
	netcode_address_t get_opened_address() const;
};
