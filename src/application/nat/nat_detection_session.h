#pragma once
#include <functional>
#include <optional>
#include <future>

#include "3rdparty/yojimbo/netcode.io/netcode.h"

#include "application/network/address_and_port.h"
#include "application/nat/nat_traversal_settings.h"
#include "application/network/resolve_address_result.h"
#include "application/nat/nat_type.h"

#include "application/nat/stun_request_structs.h"

using log_sink_type = std::function<void(const std::string&)>;

struct netcode_socket_t;

struct nat_detection_result {
	nat_type type;
	int port_delta = 0;
	port_type predicted_next_port;

	std::string describe() const;
};

using stun_counter_type = int;

class nat_detection_session {
	struct request_state {
		netcode_address_t destination;
		std::optional<netcode_address_t> translated_address;

		request_state(const netcode_address_t& destination) : destination(destination) {}

		bool completed() const {
			return translated_address != std::nullopt;
		}
	};

	struct stun_request_state : request_state {
		using request_state::request_state;

		STUNMessageHeader source_request;
	};

	net_time_t when_last_sent_packet = -1;
	net_time_t when_last_made_requests = -1;

	const nat_traversal_settings settings;
	stun_counter_type& current_stun_index;
	log_sink_type log_sink;
	std::string full_log;

	std::vector<std::future<resolve_address_result>> future_stun_hosts;
	std::future<resolve_address_result> future_port_probing_host;

	std::vector<netcode_address_t> resolved_stun_hosts;
	std::optional<netcode_address_t> resolved_port_probing_host;

	std::vector<stun_request_state> stun_requests;
	std::vector<request_state> port_probing_requests;

	void send_requests();

	std::optional<nat_detection_result> detected_nat;

	std::vector<std::pair<netcode_address_t, std::vector<std::byte>>> queued_packets;
	void send_one_packet(netcode_socket_t);
	void send_packets(netcode_socket_t);
	void analyze_results(port_type source_port);

	address_and_port get_next_stun_host();

	void advance_resolution_of_stun_hosts();
	void advance_resolution_of_port_probing_host();

	void advance_host_resolution();

	void retry_resolve_port_probing_host();

	bool enough_stun_hosts_resolved() const;
	bool all_required_hosts_resolved() const;

	void log_info(const std::string&);
	void handle_packet(const netcode_address_t& from, uint8_t* buffer, const int bytes_received);
	void receive_packets(netcode_socket_t socket);

	void finish_port_probe(const netcode_address_t& from, const netcode_address_t& result);

public:
	using session_input = ::nat_traversal_settings;

	nat_detection_session(session_input, stun_counter_type&);
	nat_detection_session(session_input, stun_counter_type&, log_sink_type);

	void advance(netcode_socket_t);

	std::optional<nat_detection_result> query_result() const;

	const std::string& get_full_log() const;
};
