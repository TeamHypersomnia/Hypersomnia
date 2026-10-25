#pragma once
#include <chrono>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include "augs/string/parse_url.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/network/netcode_socket_raii.h"
#include "augs/network/netcode_utils.h"
#include "application/network/address_utils.h"
#include "application/network/host_with_default_port.h"
#include "application/masterserver/masterserver_commands.h"
#include "application/main/server_key.h"
#include "application/config_json_table.h"

/*
	--print-join-string: what a server's players paste to add it to their favorites - "<address>,<name>,0x<key>"
	(connect_target::find_server_identifiers), one line on stdout; everything else on stderr.

	The address: the public one, as the server list sees this machine (one tell_me_my_address, 3 seconds at most - also
	with the server off the list, nothing is listed). No answer: this machine's own address - quietly when it's public
	(a VPS), with a warning when it's only one on its network (a LAN, or the server list down).
*/

namespace server_join_string {
	/*
		Where the line goes: stdout as it was before main sent it, and with it the logs, to stderr.
	*/

	inline std::streambuf* line_out = nullptr;

	/*
		This machine's public IP as the server list sees it, or nothing in 3 seconds.
	*/

	inline std::optional<netcode_address_t> ask_public_address(const std::string& server_list) {
		const auto parsed = parsed_url(server_list);

		if (!parsed.valid()) {
			return std::nullopt;
		}

		const auto resolved = resolve_address({ parsed.host, port_type(parsed.port) });

		if (resolved.result != resolve_result_type::OK) {
			return std::nullopt;
		}

		try {
			auto socket = netcode_socket_raii();
			auto destination = resolved.addr;

			const auto stamp = static_cast<double>(std::chrono::steady_clock::now().time_since_epoch().count());
			const auto bytes = augs::to_bytes(masterserver_request(masterserver_in::tell_me_my_address { stamp }));

			std::optional<netcode_address_t> answer;
			const auto until = std::chrono::steady_clock::now() + std::chrono::seconds(3);

			/*
				Sent again every half a second - a lost packet doesn't cost the whole wait.
			*/

			auto next_send = std::chrono::steady_clock::now();

			while (!answer.has_value() && std::chrono::steady_clock::now() < until) {
				if (std::chrono::steady_clock::now() >= next_send) {
					netcode_socket_send_packet(&socket.socket, &destination, const_cast<std::byte*>(bytes.data()), static_cast<int>(bytes.size()));
					next_send += std::chrono::milliseconds(500);
				}

				receive_netcode_packets(socket.socket, [&](const netcode_address_t& from, const uint8_t* const data, const int size) {
					if (from != destination) {
						return;
					}

					try {
						const auto response = augs::from_bytes<masterserver_response>(reinterpret_cast<const std::byte*>(data), size);

						if (const auto mine = std::get_if<masterserver_out::tell_me_my_address>(&response)) {
							if (!std::memcmp(&mine->session_timestamp, &stamp, sizeof(stamp))) {
								answer = mine->address;
							}
						}
					}
					catch (const augs::stream_read_error&) {}
				});

				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}

			return answer;
		}
		catch (const netcode_socket_raii_error&) {
			return std::nullopt;
		}
	}

	/*
		Just the IP of an address - its port is the socket's, not the game's.
	*/

	inline std::string ip_of(netcode_address_t address) {
		address.port = 0;
		const auto text = ::ToString(address);

		/*
			ToString writes a zero port as ":0" - the IP alone.
		*/

		if (const auto colon = text.rfind(":0"); colon != std::string::npos && colon + 2 == text.size()) {
			return text.substr(0, colon);
		}

		return text;
	}

	/*
		Exit code. The line on stdout, how it was made on stderr.
	*/

	inline int print(const config_json_table& config) {
		if (const auto error = server_key::dedicated_start_error()) {
			std::cerr << "ERROR: " << *error << std::endl;
			return EXIT_FAILURE;
		}

		const auto seed = server_key::load_or_create(server_key::path());
		const auto key_hex = seed.has_value() ? server_key::public_key_hex(*seed) : std::string();

		if (key_hex.empty()) {
			std::cerr << "ERROR: Could not read or make " << server_key::path().string() << std::endl;
			return EXIT_FAILURE;
		}

		/*
			A dedicated server with no name is "My server" (server_setup) - and the default port is left out.
		*/

		const auto configured_name = std::string(config.server.server_name);
		const auto name = configured_name.empty() ? std::string("My server") : configured_name;
		const auto port = config.server_start.port;
		const auto port_suffix = port == DEFAULT_GAME_PORT_V ? std::string() : ":" + std::to_string(port);

		/*
			Off the list (no notified_server_list): the default one is asked anyway - it answers anyone who asks.
		*/

		const auto configured_list = std::string(config.server.notified_server_list);
		const auto server_list = configured_list.empty() ? std::string("masterserver.hypersomnia.io:8430") : configured_list;
		const auto public_address = ask_public_address(server_list);

		auto ip = std::string();

		if (public_address.has_value()) {
			ip = ip_of(*public_address);
		}
		else if (const auto own = get_internal_network_address(); own.has_value()) {
			ip = ip_of(*own);

			if (::is_internal(*own)) {
				std::cerr << "Couldn't learn the public address - " << server_list << " didn't answer. " << ip << " is this machine's address on its network: fine for a LAN; over the internet, put your public address instead." << std::endl;
			}
		}
		else {
			ip = "<your server's address>";
			std::cerr << "Couldn't learn any address of this machine - put yours in." << std::endl;
		}

		std::cerr << "Publish this line - players who copy it get your server added to their favorites:" << std::endl;
		auto out = std::ostream(line_out != nullptr ? line_out : std::cout.rdbuf());
		out << ip << port_suffix << "," << name << ",0x" << key_hex << std::endl;
		return EXIT_SUCCESS;
	}
}
