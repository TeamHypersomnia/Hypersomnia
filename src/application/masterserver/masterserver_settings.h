#pragma once
#include "augs/network/port_type.h"

struct masterserver_settings {
	// GEN INTROSPECTOR struct masterserver_settings
	std::string ip = "127.0.0.1";
	augs::path_type ssl_cert_path;
	augs::path_type ssl_private_key_path;
	augs::path_type signalling_ssl_cert_path;
	augs::path_type signalling_ssl_private_key_path;
	unsigned server_entry_timeout_secs = 60;
	unsigned suppress_community_server_webhooks_after_launch_for_secs = 20;

	unsigned signalling_peer_timeout_secs = 25;
	std::string signalling_server_bind_address = "0.0.0.0";
	port_type signalling_server_port = 8000;
	port_type first_udp_command_port = DEFAULT_MASTERSERVER_PORT_V;
	int num_udp_command_ports = 5;

	port_type server_list_port = 8420;
	port_type fallback_http_server_list_port = 8410;

	augs::path_type cert_pem_path;
	augs::path_type key_pem_path;

	float sleep_ms = 8;

	bool report_rtc_errors_to_webhook = false;

	std::vector<std::string> official_hosts;
	// END GEN INTROSPECTOR

	port_type get_last_udp_command_port() const {
		return first_udp_command_port + num_udp_command_ports - 1;
	}

	bool operator==(const masterserver_settings& b) const = default;
};
