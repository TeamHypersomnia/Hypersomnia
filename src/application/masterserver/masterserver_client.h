#pragma once

enum class server_type : uint8_t {
	NATIVE,
	WEB
};

struct masterserver_entry_meta {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct masterserver_entry_meta
	double time_hosted = 0.0;
	host_string_type official_url;
	webrtc_id_type webrtc_id;
	server_type type = server_type::NATIVE;
	// END GEN INTROSPECTOR

	bool is_community_server() const {
		return official_url.empty();
	}

	bool is_official_server() const {
		return !official_url.empty();
	}
};

struct masterserver_client {
	double time_last_heartbeat = -1;

	masterserver_entry_meta meta;
	server_heartbeat last_heartbeat;
};

using server_list_type = std::unordered_map<webrtc_id_type, masterserver_client>;

