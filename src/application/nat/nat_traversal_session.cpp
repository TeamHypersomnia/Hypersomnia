#include "augs/network/netcode_utils.h"
#include "augs/network/netcode_sockets.h"

#include "application/nat/nat_traversal_session.h"
#include "application/masterserver/masterserver.h"
#include "application/masterserver/masterserver_requests.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/to_bytes.h"

using traversal_step = masterserver_in::nat_traversal_step;

double yojimbo_time();

nat_traversal_session::nat_traversal_session(const nat_traversal_input& input) 
	: input(input), when_began(yojimbo_time())
{
	log_info("---- BEGIN NAT TRAVERSAL ----");
	log_info("Begin traversing %x.", ::ToString(input.traversed_address));
}

static bool symmetric(const nat_type t) {
	return t >= nat_type::ADDRESS_SENSITIVE;
}

static bool conic(const nat_type t) {
	return t <= nat_type::CONE;
}

void nat_traversal_session::advance_init() {

}

bool nat_traversal_session::timed_out() {
	if (current_state == state::TIMED_OUT) {
		return true;
	}

	const auto& settings = input.traversal_settings;
	const auto total_timeout_secs = settings.total_timeout_secs;

	if (try_fire_interval(total_timeout_secs, when_began)) {
		log_info("Failed to traverse NAT: timed out (%xs).", total_timeout_secs);
		log_info("---- FINISH NAT TRAVERSAL ----");

		current_state = state::TIMED_OUT;
		return true;
	}

	return false;
}

void nat_traversal_session::advance(const netcode_socket_t& socket) {
	if (current_state == state::TRAVERSAL_COMPLETE) {
		return;
	}

	if (timed_out()) {
		return;
	}

	receive_packets(socket);

	auto request = [&](const auto& step) {
		netcode_send_to_masterserver(socket, input.masterserver_address, step);

		if (requests_fired % 5 == 0) {
			log_info("Firing requests.");
		}

		++requests_fired;
	};

	auto ping_target_host = [&](std::optional<port_type> override_port = std::nullopt) {
		auto target_address = input.traversed_address;

		if (override_port != std::nullopt) {
			target_address.port = *override_port;
		}

		ping_this_server(socket, target_address, -1);
	};
	
	if (conic(input.client.type) && conic(input.server.type)) {
		if (current_state == state::INIT) {
			current_state = state::TRAVERSING;
		}

		if (current_state == state::TRAVERSING) {
			if (try_fire_interval(input.traversal_settings.request_interval_ms / 1000.0, when_last_sent_request)) {
				auto step = traversal_step();
				step.target_server = input.traversed_address;

				/* 
					This will take the port with which the request 
					arrives at the masterserver.
				*/

				step.source_external_port = 0;

				request(step);
				ping_target_host();
			}
		}

		return;
	}

	if (symmetric(input.client.type)) {

	}
}

void nat_traversal_session::handle_packet(const netcode_address_t& from, uint8_t* packet_buffer, const int packet_bytes) {
	if (host_equal(from, input.traversed_address)) {
		log_info("Successfully traversed the host.\nSuccess packet came from: %x", ::ToString(from));
		log_info("---- FINISH NAT TRAVERSAL ----");

		current_state = state::TRAVERSAL_COMPLETE;
		opened_port = from.port;

		return;
	}

	(void)packet_buffer;
	(void)packet_bytes;
}

void nat_traversal_session::receive_packets(netcode_socket_t socket) {
	auto handle = [&](auto&&... args) {
		handle_packet(std::forward<decltype(args)>(args)...);
	};

	::receive_netcode_packets(socket, handle);
}

port_type nat_traversal_session::get_opened_port_at_target_host() const {
	return opened_port;
}

const std::string& nat_traversal_session::get_full_log() const {
	return full_log;
}

nat_traversal_session::state nat_traversal_session::get_current_state() const {
	return current_state;
}

netcode_address_t nat_traversal_session::get_opened_address() const {
	auto result = input.traversed_address;
	result.port = get_opened_port_at_target_host();
	return result;
}
