#include "augs/network/netcode_utils.h"
#include "augs/network/netcode_sockets.h"

#include "application/nat/nat_traversal_session.h"
#include "application/masterserver/masterserver.h"
#include "application/masterserver/masterserver_requests.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/to_bytes.h"
#include "application/network/resolve_address.h"
#include "augs/templates/thread_templates.h"
#include "application/nat/stun_request.h"
#include "augs/network/netcode_utils.h"
#include "augs/templates/bit_cast.h"
#include "augs/misc/time_utils.h"
#include "application/masterserver/gameserver_command_readwrite.h"
#include "application/nat/stun_server_provider.h"

using traversal_step = masterserver_in::nat_traversal_step;

std::string nat_traversal_state_to_string(const nat_traversal_session::state state);
std::string nat_type_to_string(const nat_type type);

double yojimbo_time();

nat_traversal_session::nat_traversal_session(const nat_traversal_input& input, stun_server_provider& stun_provider) : 
	input(input), 
	session_guid(augs::date_time().secs_since_epoch()),
	when_began(yojimbo_time()),
	chosen_masterserver_port_probe(stun_provider.get_next_port_probe(input.detection_settings.port_probing).default_port)
{
	log_info("---- BEGIN NAT TRAVERSAL ----");
	log_info("Begin traversing <%x>.", ::ToString(input.traversed_address));
	log_info("Session timestamp: %f", session_guid);

	log_info("Client NAT: %x", nat_type_to_string(input.client.type));
	log_info("Server NAT: %x", nat_type_to_string(input.server.type));
}

void nat_traversal_session::set(const state new_state) {
	if (new_state == traversal_state) {
		log_info("[%x] (Re-entered)", nat_traversal_state_to_string(new_state));
	}
	else {
		log_info("[%x] -> [%x]", nat_traversal_state_to_string(traversal_state), nat_traversal_state_to_string(new_state));
	}

	traversal_state = new_state;
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
	if (traversal_state == state::TIMED_OUT) {
		return true;
	}

	const auto& settings = input.traversal_settings;
	const auto traversal_attempt_timeout_secs = settings.traversal_attempt_timeout_secs;

	if (try_fire_interval(traversal_attempt_timeout_secs, when_began)) {
		log_info("Failed to traverse NAT: timed out (%xs).", traversal_attempt_timeout_secs);
		set(state::TIMED_OUT);
		log_info("---- FINISH NAT TRAVERSAL ----");
		return true;
	}

	return false;
}

netcode_address_t nat_traversal_session::get_current_masterserver_address() const {
	auto target_address = input.masterserver_address;
	target_address.port = chosen_masterserver_port_probe;

	return target_address;
}

void nat_traversal_session::advance(const netcode_socket_t& socket) {
	if (traversal_state == state::TRAVERSAL_COMPLETE) {
		return;
	}

	if (timed_out()) {
		return;
	}

	const auto packet_interval_secs = input.detection_settings.packet_interval_ms / 1000.0;
	const auto request_interval_secs = input.detection_settings.request_interval_ms / 1000.0;

	packet_queue.send_some(socket, packet_interval_secs, [&](const std::string& l) { log_info(l); });
	receive_packets(socket);

	const auto client_type = input.client.type;
	const auto server_type = input.server.type;

	const bool both_SYMPS = 
		server_type == nat_type::PORT_SENSITIVE 
		&& client_type == nat_type::PORT_SENSITIVE
	;

	auto make_traversal_step = [&]() {
		const auto port_dt = input.client.port_delta;

		auto step = traversal_step();
		step.target_server = input.traversed_address;

		auto& payload = step.payload;
		payload.session_guid = session_guid;
		payload.source_port_delta = port_dt;
		payload.ping_back_at_multiple_ports = symmetric(client_type);

		if (both_SYMPS) {
			/* Give it a break. Brute-force very unlikely to work in this case. */
			payload.ping_back_at_multiple_ports = false;
		}

		return step;
	};

	auto request_masterserver = [&](const auto& step) {
		const auto payload = augs::to_bytes(masterserver_request(step));
		packet_queue(get_current_masterserver_address(), payload);

		if (requests_fired % 5 == 0) {
			log_info("Firing requests.");
		}

		++requests_fired;
	};

	auto reset_request_timer = [&]() {
		when_last_masterserver_request = -1;
	};

	auto open_holes_for_server = [&]() {
		const auto& settings = input.traversal_settings;

		client_holes_opened = true;

		auto open_hole_directed_at = [&](const port_type target_port, bool short_ttl = false) {
			auto target_address = input.traversed_address;
			target_address.port = target_port;

			const auto sequence = augs::bit_cast<std::uint64_t>(session_guid);

			auto packet = make_ping_packet(target_address, sequence);

			if (short_ttl) {
				if (settings.short_ttl.is_enabled) {
					packet.ttl = settings.short_ttl.value;
				}
			}

			packet_queue(packet);
		};

		if (last_server_stunned_port != 0) {
			const auto port_dt = input.server.port_delta;
			const auto first_predicted_port = port_dt + last_server_stunned_port;

			open_hole_directed_at(first_predicted_port);

			if (both_SYMPS) {
				/* Give it a break. Brute-force very unlikely to work in this case. */
			}
			else {
				for (int i = 1; i <= settings.num_brute_force_packets; ++i) {
					const auto offset = i * port_dt;
					open_hole_directed_at(offset + first_predicted_port, true);
				}
			}
		}
		else {
			open_hole_directed_at(input.traversed_address.port);
		}
	};

	auto start_requesting_remote_port_info = [&]() {
		set(state::REQUESTING_REMOTE_PORT_INFO);
		reset_request_timer();
	};

	auto try_request_interval = [&]() {
		return try_fire_interval(request_interval_secs, when_last_masterserver_request);
	};

	/* Advance operations in progress */

	if (traversal_state == state::REQUESTING_REMOTE_PORT_INFO) {
		if (try_request_interval()) {
			auto step = make_traversal_step();
			step.payload.type = nat_traversal_step_type::PORT_RESOLUTION_REQUEST;

			request_masterserver(step);
		}

		return;
	}

	/* Flow */

	do {
		switch (traversal_state) {
			case state::INIT:
				if (conic(server_type)) {
					set(state::SERVER_STUN_REQUIREMENT_MET);
					continue;
				}
				else {
					start_requesting_remote_port_info();
					break;
				}

			case state::SERVER_STUN_REQUIREMENT_MET:
				set(state::TRAVERSING);
				continue;

			case state::TRAVERSING: {
				if (try_request_interval()) {
					open_holes_for_server();
					request_masterserver(make_traversal_step());
				}

				break;
			}

			case state::REQUESTING_REMOTE_PORT_INFO:
			case state::TRAVERSAL_COMPLETE:
			case state::TIMED_OUT:
			default:
				break;
		}
	} while(false);
}

void nat_traversal_session::handle_packet(const netcode_address_t& from, uint8_t* packet_buffer, const int packet_bytes) {
	if (traversal_state == state::TRAVERSAL_COMPLETE) {
		return;
	}

	if (stun_in_progress.has_value()) {
		const auto bytes = reinterpret_cast<const std::byte*>(packet_buffer);

		if (stun_in_progress->handle_packet(bytes, packet_bytes)) {
			return;
		}
	}

	if (from == get_current_masterserver_address()) {
		auto handle_server_stun_result = [this](
			const auto& response
		) {
			using R = remove_cref<decltype(response)>;

			if constexpr(std::is_same_v<R, masterserver_out::stun_result_info>) {
				if (session_guid == response.session_guid) {
					last_server_stunned_port = response.resolved_external_port;
					set(state::SERVER_STUN_REQUIREMENT_MET);

					const auto dt = input.server.port_delta;

					log_info("stun_result_info arrived. Server resolved port: %x", last_server_stunned_port);
					log_info("Server port delta is %x, so predicting the next open server port to be %x", dt, dt + last_server_stunned_port);
				}
				else {
					log_info("stun_result_info arrived, but the session timestamp does not match: %f", response.session_guid);
				}
			}
		};

		try {
			const auto response = augs::from_bytes<masterserver_response>(packet_buffer, packet_bytes);

			if (traversal_state == state::REQUESTING_REMOTE_PORT_INFO) {
				std::visit(handle_server_stun_result, response);
			}
		}
		catch (const augs::stream_read_error& err) {

		}
	}
	else if (host_equal(from, input.traversed_address)) {
		if (const auto maybe_session_guid = read_nat_traversal_success_packet(packet_buffer, packet_bytes)) {

			if (*maybe_session_guid == session_guid) {
				log_info("Success packet arrived from: <%x> (session timestamp: %f)", ::ToString(from), *maybe_session_guid);
				log_info("Successfully traversed the host.");
				set(state::TRAVERSAL_COMPLETE);
				log_info("---- FINISH NAT TRAVERSAL ----");

				opened_remote_port = from.port;
			}
			else {
				log_info("Success packet arrived from the host, but the session timestamp does not match: %f", *maybe_session_guid);
			}
		}

		return;
	}
}

void nat_traversal_session::receive_packets(netcode_socket_t socket) {
	auto handle = [&](auto&&... args) {
		handle_packet(std::forward<decltype(args)>(args)...);
	};

	::receive_netcode_packets(socket, handle);
}

port_type nat_traversal_session::get_opened_port_at_target_host() const {
	return opened_remote_port;
}

const std::string& nat_traversal_session::get_full_log() const {
	return full_log;
}

nat_traversal_session::state nat_traversal_session::get_current_state() const {
	return traversal_state;
}

netcode_address_t nat_traversal_session::get_opened_address() const {
	auto result = input.traversed_address;
	result.port = get_opened_port_at_target_host();
	return result;
}

stun_session::stun_session(
	const address_and_port& host,
	log_function log_info
) : 
	future_stun_host(async_resolve_address(host)), 
	when_began(yojimbo_time()),
	log_info(log_info),
	host(host)
{

}

std::optional<netcode_queued_packet> stun_session::advance(const double request_interval_secs, randomization& rng) {
	if (stun_host.has_value()) {
		return std::nullopt;
	}

	auto& future = future_stun_host;

	if (valid_and_is_ready(future)) {
		auto result = future.get();

		log_info(result.report());

		if (result.result == resolve_result_type::OK) {
			stun_host = result.addr;
			log_info("Preparing STUN request headers.");
			source_request = make_stun_request(rng);
		}
	}

	if (stun_host.has_value()) {
		if (try_fire_interval(request_interval_secs, when_generated_last_packet)) {
			when_sent_first_request = yojimbo_time();
			return netcode_queued_packet { *stun_host, augs::to_bytes(source_request) };
		}
	}

	return std::nullopt;
}

stun_session::state stun_session::get_current_state() const {
	if (external_address.has_value()) {
		return state::COMPLETED;
	}

	if (!future_stun_host.valid()) {
		if (stun_host.has_value()) {
			return state::SENDING_REQUEST_PACKETS;
		}
		else {
			return state::COULD_NOT_RESOLVE_STUN_HOST;
		}
	}
	else {
		return state::RESOLVING_STUN_HOST;
	}
}

bool stun_session::has_timed_out(const double timeout_secs) const { 
	if (get_current_state() == stun_session::state::SENDING_REQUEST_PACKETS) {
		return yojimbo_time() - when_began >= timeout_secs;
	}

	return false;
}

std::optional<netcode_address_t> stun_session::query_result() const {
	return external_address;
}

bool stun_session::handle_packet(const std::byte* const packet_buffer, const int num_bytes_received) {
	if (const auto translated = read_stun_response(source_request, packet_buffer, num_bytes_received)) {
		if (stun_host.has_value()) {
			log_info(typesafe_sprintf("received STUN response: <%x> -> <%x>", ::ToString(*stun_host), ::ToString(*translated)));
		}

		when_completed = yojimbo_time();
		external_address = *translated;

		return true;
	}

	return false;
}

const std::optional<netcode_address_t>& stun_session::get_resolved_stun_host() const {
	return stun_host;
}

double stun_session::get_ping_seconds() const {
	return when_completed - when_sent_first_request;
}

std::string nat_traversal_state_to_string(const nat_traversal_session::state state) {
	using S = nat_traversal_session::state;

	switch (state) {
		case S::INIT:
			return "Initializing";
		case S::TRAVERSING:
			return "Traversing";
		case S::SERVER_STUN_REQUIREMENT_MET:
			return "Server STUN requirement met";
		case S::REQUESTING_REMOTE_PORT_INFO:
			return "Requesting remote port info";
		case S::TRAVERSAL_COMPLETE:
			return "Traversal complete";
		case S::TIMED_OUT:
			return "Traversal timed out";

		default:
			return "Unknown";
	}
}

rgba nat_traversal_state_to_color(const nat_traversal_session::state state) {
	using S = nat_traversal_session::state;

	switch (state) {
		case S::INIT:
			return orange;
		case S::SERVER_STUN_REQUIREMENT_MET:
			return cyan;
		case S::REQUESTING_REMOTE_PORT_INFO:
			return yellow;
		case S::TRAVERSING:
			return cyan;
		case S::TRAVERSAL_COMPLETE:
			return green;
		case S::TIMED_OUT:
			return red;

		default:
			return red;
	}
}


std::string censor_ips(std::string text) {
#if !IS_PRODUCTION_BUILD
	return text;
#endif

	bool censoring = false;

	for (std::size_t i = 0; i < text.size(); ++i) {
		if (text[i] == '<') {
			censoring = true;
		}
		else if (text[i] == '>') {
			censoring = false;
		}
		else if (censoring) {
			text[i] = '*';
		}
	}

	return text;
}

