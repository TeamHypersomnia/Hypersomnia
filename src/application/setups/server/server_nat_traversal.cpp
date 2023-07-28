#include "application/masterserver/netcode_address_hash.h"
#include "application/setups/server/server_nat_traversal.h"
#include "application/nat/stun_session.h"
#include "augs/templates/container_templates.h"
#include "application/masterserver/gameserver_commands.h"
#include "augs/readwrite/to_bytes.h"
#include "application/masterserver/gameserver_command_readwrite.h"
#include "augs/network/netcode_utils.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/templates/bit_cast.h"
#include "application/masterserver/masterserver.h"
#include "application/nat/stun_server_provider.h"

double yojimbo_time();

server_nat_traversal::server_nat_traversal(
	const server_nat_traversal_input& input,
	const std::optional<netcode_address_t>& masterserver_address
) : 
	masterserver_address(masterserver_address),
	input(input)
{}

void server_nat_traversal::send_packets(netcode_socket_t socket) {
	const auto interval_secs = input.detection_settings.packet_interval_ms / 1000.0;
	packet_queue.send_some(socket, interval_secs, no_LOG());
}

void server_nat_traversal::session::relay_stun_result(
	netcode_address_t masterserver_address,
	netcode_packet_queue& queue
) {
	if (stun.has_value()) {
		if (const auto result = stun->query_result()) {
			auto result_info = masterserver_in::stun_result_info();

			result_info.session_guid = session_guid;
			result_info.client_origin = request.client_origin;
			result_info.resolved_external_port = result->port;

			const auto info_bytes = augs::to_bytes(masterserver_request(result_info));
			queue(masterserver_address, info_bytes);
			++times_sent_port_info;
		}
	}
}

void server_nat_traversal::session::relay_port_direct(
	netcode_address_t masterserver_address,
	netcode_packet_queue& queue
) {
	auto result_info = masterserver_in::stun_result_info();

	result_info.session_guid = session_guid;
	result_info.client_origin = request.client_origin;

	/*
		Use the masterserver-visible port
	*/

	result_info.resolved_external_port = 0;

	const auto info_bytes = augs::to_bytes(masterserver_request(result_info));

	masterserver_address.port = chosen_port_probe;
	queue(masterserver_address, info_bytes);
	++times_sent_port_info;
}

void server_nat_traversal::advance() {
	const auto request_interval_secs = input.detection_settings.request_interval_ms / 1000.0;
	const auto stun_timeout_secs = input.detection_settings.stun_session_timeout_ms / 1000.0;

	erase_if(traversals, [&](auto& entry) {
		const auto client_address = entry.first;
		auto& traversal = entry.second;

		if (try_fire_interval(input.traversal_settings.traversal_attempt_timeout_secs, traversal.when_appeared)) {
			return true;
		}

		auto& stun = traversal.stun;

		if (stun.has_value()) {
			if (stun->has_timed_out(stun_timeout_secs)) {
				if (!traversal.has_stun_timed_out) {
					traversal.has_stun_timed_out = true;
					LOG("Stun request has timed out.");
				}

				return false;
			}

			if (const auto request_packet = stun->advance(request_interval_secs, stun_rng)) {
				LOG("Sending a STUN request packet.");
				packet_queue(*request_packet);

				if (!traversal.holes_opened) {
					LOG("Opening holes for the client right away to increase the chance of an adjacent port.");
					traversal.open_holes(input.traversal_settings, client_address, packet_queue);
				}
			}

			const auto state = stun->get_current_state();

			if (state == stun_session::state::COMPLETED) {
				if (masterserver_address.has_value()) {
					const auto& times = traversal.times_sent_port_info;

					if (times == 0) {
						LOG("Sending the STUN result info for the first time.");

						traversal.relay_stun_result(
							*masterserver_address, 
							packet_queue
						);
					}
				}
			}
			else if (state == stun_session::state::COULD_NOT_RESOLVE_STUN_HOST) {
				LOG("Could not resolve STUN host: %x. Picking the next one.", stun->host.address);
				relaunch(stun);
			}
		}

		return false;
	});
}

void server_nat_traversal::relaunch(std::optional<stun_session>& stun_session) {
	stun_session.reset();
	const auto next_host = input.stun_provider.get_next();

	auto log_info = [](const std::string& s) { LOG(s); };
	stun_session.emplace(next_host, log_info);
}

bool server_nat_traversal::handle_auxiliary_command(
	const netcode_address_t& from,
	const std::byte* packet_buffer,
	const std::size_t packet_bytes
) {
	if (handle_stun_packet(packet_buffer, packet_bytes)) {
		return true;
	}

	if (packet_bytes < 1) {
		return false;
	}

	const bool is_auxiliary_cmd = packet_buffer[0] == static_cast<std::byte>(NETCODE_AUXILIARY_COMMAND_PACKET);

	if (!is_auxiliary_cmd) {
		return false;
	}

	auto handle = [&](const auto& typed_request) {
		using T = remove_cref<decltype(typed_request)>;

		if constexpr(std::is_same_v<T, gameserver_ping_request>) {
			/* Handled elsewhere */
			return;
		}
		else if constexpr(std::is_same_v<T, masterserver_out::nat_traversal_step>) {
			if (last_detected_nat.type == nat_type::PUBLIC_INTERNET) {
				return;
			}

			if (from != masterserver_address) {
				return;
			}

			LOG("Received traversal step from masterserver.");

			const auto masterserver_visible_client_address = typed_request.client_origin.address;

			const auto predicted_open_client_address = [&]() {
				auto origin_address = masterserver_visible_client_address;
				origin_address.port += typed_request.payload.source_port_delta;

				return origin_address;
			}();

			const auto& client_address = predicted_open_client_address;
			const auto& payload = typed_request.payload;

			auto create_fresh_entry = [&]() {
				traversals.erase(client_address);

				auto traversal = session();
				traversal.session_guid = payload.session_guid;

				LOG("Creating fresh entry for %x. Session timestamp: %f", ::ToString(client_address), traversal.session_guid);
				traversals.try_emplace(client_address, std::move(traversal));
			};

			if (auto entry = mapped_or_nullptr(traversals, client_address)) {
				if (payload.session_guid < entry->session_guid) {
					return;
				}

				if (payload.session_guid > entry->session_guid) {
					create_fresh_entry();
				}
			}
			else {
				const auto sane_limit_traversals = 30;

				if (traversals.size() < sane_limit_traversals) {
					create_fresh_entry();
				}
				else {
					LOG("Too many traversal entries for now. Ignoring request.");
				}
			}

			auto& traversal = traversals[client_address];
			traversal.request = typed_request;

			if (last_detected_nat.type == nat_type::PORT_SENSITIVE) {
				traversal.chosen_port_probe = input.stun_provider.get_next_port_probe(input.detection_settings.port_probing).default_port;
			}

			switch (payload.type) {
				case nat_traversal_step_type::PORT_RESOLUTION_REQUEST: {
					const auto max_times_resend_port_info = 5;

					if (traversal.times_sent_port_info > max_times_resend_port_info) {
						break;
					}

					if (masterserver_address == std::nullopt) {
						break;
					}

					if (last_detected_nat.type == nat_type::ADDRESS_SENSITIVE) {
						auto& stun = traversal.stun;

						if (stun.has_value()) {
							const auto state = stun->get_current_state();

							if (state == stun_session::state::COMPLETED) {
								LOG("Resending the STUN result.");

								traversal.relay_stun_result(
									*masterserver_address,
									packet_queue
								);
							}
							else {
								// LOG("Already launched a STUN session in this traversal. Ignoring further requests until STUN completes.");
							}

							break;
						}

						LOG("Launching a STUN session for this traversal.");
						relaunch(stun);
					}
					else if (last_detected_nat.type == nat_type::PORT_SENSITIVE) {
						traversal.relay_port_direct(
							*masterserver_address,
							packet_queue
						);

						if (!traversal.holes_opened) {
							LOG("Opening holes for the client right away to increase the chance of an adjacent port.");
							traversal.open_holes(input.traversal_settings, client_address, packet_queue);
						}
					}

					break;
				}

				case nat_traversal_step_type::PINGBACK:
					traversal.open_holes(input.traversal_settings, client_address, packet_queue);
					break;

				default:
					break;
			}
		}
		else {
			static_assert(always_false_v<T>, "No command handler for this type");
		}
	};

	try {
		const auto request = read_gameserver_command(packet_buffer, packet_bytes);
		std::visit(handle, request);
		return true;
	}
	catch (const augs::stream_read_error&) {

	}

	return false;
}

server_nat_traversal::session::session() 
	: when_appeared(yojimbo_time()) 
{}

void server_nat_traversal::session::open_holes(
	const nat_traversal_settings& settings,
	const netcode_address_t predicted_open_address,
	netcode_packet_queue& queue
) {
	auto response = gameserver_nat_traversal_response_packet();
	response.session_guid = session_guid;

	const auto port_dt = request.payload.source_port_delta;

	LOG("Opening NAT holes for the predicted client address: %x\nPort dt: %x", ::ToString(predicted_open_address), port_dt);

	const auto ping_bytes = augs::to_bytes(response);

	queue(predicted_open_address, ping_bytes);

	if (request.payload.ping_back_at_multiple_ports) {
		if (!holes_opened) {
			LOG("Pinging at multiple ports to increase the chance of success.");

			const auto first_predicted_port = predicted_open_address.port;

			for (int i = 1; i <= settings.num_brute_force_packets; ++i) {
				const auto offset = i * port_dt;
				const auto next_port = offset + first_predicted_port;

				auto next_predicted_address = predicted_open_address;
				next_predicted_address.port = next_port;

				queue(next_predicted_address, ping_bytes);
			}
		}
	}

	holes_opened = true;
}

bool server_nat_traversal::handle_stun_packet(const std::byte* packet_buffer, const std::size_t packet_bytes) {
	if (!is_behind_nat()) {
		return false;
	}

	for (auto& entry : traversals) {
		auto& stun = entry.second.stun;

		if (stun.has_value()) {
			if (stun->handle_packet(packet_buffer, packet_bytes)) {
				return true;
			}
		}
	}

	return false;
}

bool server_nat_traversal::is_behind_nat() const {
	return last_detected_nat.type != nat_type::PUBLIC_INTERNET;
}

