#pragma once
#include <future>
#include <optional>
#include "application/network/resolve_address_result.h"
#include "application/nat/stun_request_structs.h"
#include "application/network/address_and_port.h"
#include "augs/network/network_types.h"
#include "application/nat/netcode_packet_queue.h"
#include "augs/misc/log_function.h"
#include "augs/network/netcode_queued_packet.h"

struct randomization;

class stun_session {
	std::future<resolve_address_result> future_stun_host;
	std::optional<netcode_address_t> stun_host;
	std::optional<netcode_address_t> external_address;

	STUNMessageHeader source_request;

	net_time_t when_began;
	net_time_t when_sent_first_request = -1;
	net_time_t when_completed = -1;
	net_time_t when_generated_last_packet = -1;

	log_function log_info;

public: 
	const address_and_port host;

	enum class state {
		RESOLVING_STUN_HOST,
		COULD_NOT_RESOLVE_STUN_HOST,
		SENDING_REQUEST_PACKETS,
		COMPLETED
	};

	stun_session(const address_and_port& host, log_function log_info);

	std::optional<netcode_address_t> query_result() const;

	state get_current_state() const;
	bool has_timed_out(double timeout_secs) const;
	
	std::optional<netcode_queued_packet> advance(double request_interval_secs, randomization& rng);
	bool handle_packet(const std::byte* buffer, const int bytes_received);

	const std::optional<netcode_address_t>& get_resolved_stun_host() const;
	double get_ping_seconds() const;
};
