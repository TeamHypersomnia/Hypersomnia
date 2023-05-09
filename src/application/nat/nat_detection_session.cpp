#include <random>
#include <numeric>
#include "application/nat/nat_detection_session.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/log.h"
#include "application/network/resolve_address.h"
#include "augs/templates/thread_templates.h"
#include "augs/network/netcode_sockets.h"
#include "augs/network/netcode_utils.h"
#include "application/masterserver/masterserver.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "application/masterserver/masterserver.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/nat/stun_request.h"
#include "application/nat/stun_server_provider.h"
#include "augs/misc/time_utils.h"

std::string nat_detection_result::describe() const {
	using N = nat_type;

	switch (type) {
		case N::PUBLIC_INTERNET:
			return "This computer appears to be directly in the public internet.";
		case N::PORT_PRESERVING_CONE:
			return typesafe_sprintf("Detected a port-preserving, cone-like NAT.");
		case N::CONE:
			return typesafe_sprintf("Detected a cone-like NAT.\nDelta: %x ", port_delta);
		case N::ADDRESS_SENSITIVE:
			return typesafe_sprintf("Detected a symmetric NAT.\nIt is address-sensitive. Delta: %x ", port_delta);
		case N::PORT_SENSITIVE:
			return typesafe_sprintf("Detected a symmetric NAT.\nIt is port-sensitive. Delta: %x ", port_delta);
		default: return "Unknown result.";
	}
}

address_and_port nat_detection_session::get_next_stun_host() {
	return stun_provider.get_next();
}

nat_detection_session::nat_detection_session(nat_detection_session::session_input in, stun_server_provider& stun_provider) 
	: nat_detection_session(in, stun_provider, [](const auto& s) { LOG(s); })
{}

nat_detection_session::nat_detection_session(
	nat_detection_session::session_input in,
	stun_server_provider& stun_provider,
	log_function log_sink
) :
	settings(in),
	stun_provider(stun_provider),
	log_sink(log_sink),
	future_port_probing_host(async_resolve_address(in.port_probing.host)),
	session_timestamp(augs::date_time().secs_since_epoch())
{
	log_info("---- BEGIN NAT ANALYSIS ----");

	auto n = in.num_stun_hosts_used_for_detection;

	while (n--) {
		future_stun_hosts.emplace_back(async_resolve_address(get_next_stun_host()));
	}
}

std::optional<nat_detection_result> nat_detection_session::query_result() const {
	return detected_nat;
}

void nat_detection_session::retry_resolve_port_probing_host() {
	resolved_port_probing_host.reset();
	future_port_probing_host = async_resolve_address(settings.port_probing.host);

	log_info("Retrying resolution of the port probing host.");
}

void nat_detection_session::advance_resolution_of_stun_hosts() {
	if (enough_stun_hosts_resolved()) {
		return;
	}

	auto pick_next_stun_host = [&]() -> decltype(auto) {
		return async_resolve_address(get_next_stun_host());
	};

	for (auto& future : future_stun_hosts) {
		if (valid_and_is_ready(future)) {
			auto result = future.get();

			log_info(result.report());

			if (result.result == resolve_result_type::OK) {
				resolved_stun_hosts.emplace_back(result.addr);

				if (enough_stun_hosts_resolved()) {
					log_info("All %x stun hosts resolved. Generating requests.", resolved_stun_hosts.size());

					auto rng = randomization::from_random_device();

					for (const auto& stun_host : resolved_stun_hosts) {
						auto request = stun_request_state(stun_host);
						request.source_request = make_stun_request(rng);

						stun_requests.emplace_back(std::move(request));
					}

					return;
				}
			}
			else {
				future = pick_next_stun_host();
			}
		}
	}
}


void nat_detection_session::advance_resolution_of_port_probing_host() {
	if (resolved_port_probing_host.has_value()) {
		return;
	}

	auto& target = resolved_port_probing_host;
	auto& future = future_port_probing_host;

	if (valid_and_is_ready(future)) {
		const auto result = future.get();

		log_info(result.report());

		if (result.result == resolve_result_type::OK) {
			target = result.addr;

			const auto n = settings.port_probing.num_probed_for_detection;

			for (int i = 0; i < n; ++i) {
				auto this_probe_addr = result.addr;
				this_probe_addr.port = stun_provider.get_next_port_probe(settings.port_probing).default_port;

				port_probing_requests.emplace_back(this_probe_addr);
			}

			return;
		}
		else {
			retry_resolve_port_probing_host();
		}
	}
}

bool nat_detection_session::enough_stun_hosts_resolved() const {
	return static_cast<int>(resolved_stun_hosts.size()) == settings.num_stun_hosts_used_for_detection;
}

bool nat_detection_session::all_required_hosts_resolved() const {
	return enough_stun_hosts_resolved() && resolved_port_probing_host.has_value();
}

void nat_detection_session::send_requests() {
	if (!all_required_hosts_resolved()) {
		/* Must be fired one after another. */
		return;
	}

	if (try_fire_interval(settings.request_interval_ms / 1000.0, when_last_made_requests)) {
		if (times_sent_requests % 5 == 0) {
			log_info("Firing packets for NAT resolution.");
		}

		times_sent_requests++;

		for (auto& request : stun_requests) {
			if (!request.completed()) {
				packet_queue(request.destination, augs::to_bytes(request.source_request));
			}
		}

		{
			for (auto& probation : port_probing_requests) {
				if (!probation.completed()) {
					const auto request = masterserver_in::tell_me_my_address { session_timestamp };
					auto bytes = augs::to_bytes(masterserver_request(request));

					packet_queue(probation.destination, bytes);
				}
			}
		}
	}
}

template <class F>
static nat_detection_result calculate_from(
	port_type source_port,

	const std::vector<port_type>& address_unique_translations,
	const std::vector<port_type>& port_unique_translations,

	F log_info
) {
	ensure_greater(int(address_unique_translations.size()), 0);
	ensure_greater(int(port_unique_translations.size()), 1);

	auto make_deltas = [&](const auto& ports) {
		std::vector<int> deltas;

		for (std::size_t i = 1; i < ports.size(); ++i) {
			deltas.push_back(ports[i] - ports[i - 1]);
		}

		return deltas;
	};

	const auto cross_ports_deltas = make_deltas(port_unique_translations); 
	const auto cross_address_deltas = [&]() {
		auto deltas = make_deltas(address_unique_translations); 

		const auto between_last_stun_and_port_probe = 
			address_unique_translations.back() 
			- port_unique_translations.front()
		;

		deltas.push_back(between_last_stun_and_port_probe);
		return deltas;
	}();

	const auto type = [&]() {
		const bool preserved_between_addressses = all_equal_to(cross_address_deltas, 0);
		const bool preserved_between_ports = all_equal_to(cross_ports_deltas, 0);

		if (preserved_between_addressses) {
			/* 
				It is *probably* address insensitive, so either a cone or public. 
				Ensure that it is by checking the port probes too.
			*/

			if (preserved_between_ports) {
				if (source_port == port_unique_translations[0]) {
					return nat_type::PORT_PRESERVING_CONE;
				}

				return nat_type::CONE;
			}
			else {
				log_info("WARNING! Ports preserved between destination addresses, but not between destination ports!");
				/* 
					Pathological case: 
					if for some reason the port didn't change on switching to a different destination host,
					but the port probes do determine port sensitivity, 
					perhaps we've exhausted all available ports.

					Continue calculations as if the cross address delta was non-zero.
				*/
			}

		}

		/* It is at least address sensitive */

		if (preserved_between_ports) {
			return nat_type::ADDRESS_SENSITIVE;
		}

		return nat_type::PORT_SENSITIVE;
	}();

	auto predict_port_from = [](const auto& deltas) {
		ensure(deltas.size() >= 2);

		auto result = deltas[0];

		for (std::size_t i = 1; i < deltas.size(); ++i) {
			result = std::gcd(result, deltas[i]);
		}

		return result;
	};

	const auto predicted_port_step = [&]() {
		if (type == nat_type::PORT_SENSITIVE) {
			const auto concatenated_deltas = concatenated(cross_address_deltas, cross_ports_deltas);
			return predict_port_from(concatenated_deltas);
		}

		if (type == nat_type::ADDRESS_SENSITIVE) {
			return predict_port_from(cross_address_deltas);
		}

		return 0;
	}();

	const auto last_resolved_port = port_unique_translations.back();
	const auto predicted_next_port = static_cast<port_type>(last_resolved_port + predicted_port_step);

	return nat_detection_result { type, predicted_port_step, predicted_next_port };
}

void nat_detection_session::analyze_results(const port_type source_port) {
	if (detected_nat.has_value()) {
		return;
	}

	if (!all_required_hosts_resolved()) {
		return;
	}

	auto all_complete = [](const auto& requests) {
		for (const auto& s : requests) {
			if (!s.completed()) {
				return false;
			}
		}

		return true;
	};

	if (!all_complete(stun_requests)) {
		return;
	}

	if (!all_complete(port_probing_requests)) {
		return;
	}

	auto requests_to_translated_port_list = [](const auto& requests) {
		auto ports = std::vector<port_type>();

		for (const auto& r : requests) {
			ports.emplace_back(r.translated_address->port);
		}

		return ports;
	};

	const auto address_unique_translations = requests_to_translated_port_list(stun_requests);
	const auto port_unique_translations = requests_to_translated_port_list(port_probing_requests);

	detected_nat = ::calculate_from(
		source_port, 
		address_unique_translations, 
		port_unique_translations, 
		[this](const auto& contents) { log_info(contents); }
	);

	log_info("---- FINISH NAT ANALYSIS ----");
}

const std::string& nat_detection_session::get_full_log() const {
	return full_log;
}

const nat_detection_settings& nat_detection_session::get_settings() {
	return settings;
}

void nat_detection_session::finish_port_probe(const netcode_address_t& from, const netcode_address_t& result) {
	for (auto& probation : port_probing_requests) {
		if (from == probation.destination) {
			probation.translated_address = result;
		}
	}
}

void nat_detection_session::handle_packet(const netcode_address_t& from, uint8_t* packet_buffer, const int packet_bytes) {
	auto handle_port_probe_response = [this, from](
		const auto& response
	) {
		using R = remove_cref<decltype(response)>;

		if constexpr(std::is_same_v<R, masterserver_out::tell_me_my_address>) {
			if (response.session_timestamp != session_timestamp) {
				log_info("received tell_me_my_address, but the session_timestamp does not match: %x != %x (ours)", response.session_timestamp, session_timestamp);

				return false;
			}

			const auto& our_external_address = response.address;

			log_info("port probe response: %x -> <%x>", ::ToString(from), ::ToString(our_external_address));

			finish_port_probe(from, our_external_address);

			return true;
		}

		return false;
	};

	auto try_read_masterserver_response = [&]() {
		try {
			const auto response = augs::from_bytes<masterserver_response>(packet_buffer, packet_bytes);
			return std::visit(handle_port_probe_response, response);
		}
		catch (const augs::stream_read_error& err) {

		}

		return false;
	};

	for (auto& request : stun_requests) {
		const auto bytes = reinterpret_cast<const std::byte*>(packet_buffer);

		if (const auto translated = read_stun_response(request.source_request, bytes, packet_bytes)) {
			const auto& our_external_address = *translated;

			log_info("received STUN response: %x -> <%x>", ::ToString(from), ::ToString(our_external_address));

			request.translated_address = our_external_address;
			return;
		}
	}

	if (try_read_masterserver_response()) {
		return;
	}
}

void nat_detection_session::receive_packets(netcode_socket_t socket) {
	auto handle = [&](auto&&... args) {
		handle_packet(std::forward<decltype(args)>(args)...);
	};

	::receive_netcode_packets(socket, handle);
}

void nat_detection_session::advance(netcode_socket_t socket) {
	if (detected_nat.has_value()) {
		return;
	}

	advance_resolution_of_stun_hosts();
	advance_resolution_of_port_probing_host();

	receive_packets(socket);

	send_requests();
	packet_queue.send_some(socket, settings.packet_interval_ms / 1000.0, no_LOG());
	analyze_results(socket.address.port);
}

const std::optional<netcode_address_t>& nat_detection_session::get_resolved_port_probing_host() const {
	return resolved_port_probing_host;
}

std::string stringize_bytes(const std::vector<std::byte>& bytes) {
	std::string bytes_str;

	for (const auto& b : bytes) {
		const auto bb = typesafe_sprintf("%h", int(b));

		if (bb.size() == 1) {
			bytes_str += "0";
		}

		bytes_str += bb + " ";
	}

	if (bytes_str.size() > 0) {
		bytes_str.pop_back();
	}

	return bytes_str;
}

double yojimbo_time();

std::string describe_packet(const netcode_queued_packet& p, const std::optional<int> saved_ttl) {
	return typesafe_sprintf("[PACKET%x] [%f] %x (%x bytes): %x", p.ttl ? typesafe_sprintf(" TTL = %x (def: %x)", *p.ttl, saved_ttl ? *saved_ttl : -1337) : "", yojimbo_time(), std::string("<") + ToString(p.to) + ">", p.bytes.size(), stringize_bytes(p.bytes));
}

void netcode_packet_queue::send_one(netcode_socket_t socket, log_function log_sink) {
	if (queue.empty()) {
		return;
	}

	auto packet = queue.front();
	auto& address = packet.to;
	auto& bytes = packet.bytes;

	auto saved_ttl = std::optional<int>();

	if (packet.ttl.has_value()) {
		saved_ttl = netcode_socket_get_ttl(&socket);
		netcode_socket_set_ttl(&socket, *packet.ttl);
	}

	log_sink(describe_packet(packet, saved_ttl));

	netcode_socket_send_packet(&socket, &address, bytes.data(), bytes.size());

	if (saved_ttl.has_value()) {
		netcode_socket_set_ttl(&socket, *saved_ttl);
	}

	queue.erase(queue.begin());
}

void netcode_packet_queue::send_some(netcode_socket_t socket, const double interval_seconds, log_function log_sink) {
	if (try_fire_interval(interval_seconds, when_last)) {
		send_one(socket, log_sink);
	}
}
