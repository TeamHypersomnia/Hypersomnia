#include "augs/templates/introspection_utils/introspective_equal.h"
#include "augs/templates/corresponding_field.h"

#include "augs/misc/lua/lua_utils.h"
#include "augs/readwrite/lua_file.h"
#include "augs/window_framework/window.h"

#include "application/config_lua_table.h"
#include "application/nat/stun_server_provider.h"

config_lua_table::config_lua_table(sol::state& lua, const augs::path_type& config_lua_path) {
	load(lua, config_lua_path);
}

void config_lua_table::load_patch(sol::state& lua, const augs::path_type& config_lua_path) {
	try {
		augs::load_from_lua_patch(lua, *this, config_lua_path);
	}
	catch (const augs::lua_deserialization_error& err) {
		throw config_read_error(config_lua_path, err.what());
	}
	catch (const augs::file_open_error& err) {
		throw config_read_error(config_lua_path, err.what());
	}
}

void config_lua_table::load(sol::state& lua, const augs::path_type& config_lua_path) {
	try {
		augs::load_from_lua_table(lua, *this, config_lua_path);
	}
	catch (const augs::lua_deserialization_error& err) {
		throw config_read_error(config_lua_path, err.what());
	}
	catch (const augs::file_open_error& err) {
		throw config_read_error(config_lua_path, err.what());
	}
}

void config_lua_table::save_patch(sol::state& lua, const config_lua_table& source, const augs::path_type& target_path) const {
	augs::save_as_lua_patch(lua, source, *this, target_path);
}

activity_type config_lua_table::get_last_activity() const {
	return last_activity;
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	return debug.input_recording_mode;
}

bool config_lua_table::operator==(const config_lua_table& b) const {
	return augs::introspective_equal(*this, b);
}

bool config_lua_table::operator!=(const config_lua_table& b) const {
	return !operator==(b);
}

stun_server_provider::stun_server_provider(const augs::path_type& list_file) {
	load(list_file);
}

namespace augs {
	auto file_to_lines(const augs::path_type& path) {
		auto content = file_to_string(path);
		auto s = std::stringstream(content);

		std::vector<std::string> out;

		for (std::string line; std::getline(s, line); ) {
			out.emplace_back(line);
		}

		return out;
	}
}

void stun_server_provider::load(const augs::path_type& list_file) {
	servers = augs::file_to_lines(list_file);
	usage_timestamps.resize(servers.size(), net_time_t(-1));
}

double yojimbo_time();

address_and_port stun_server_provider::get_next() {
	if (servers.empty()) {
		return {};
	}

	auto result = address_and_port();
	result.default_port = 3478;

	const auto current_server_i = current_stun_server % servers.size();

	usage_timestamps[current_server_i] = yojimbo_time();
	result.address = servers[current_server_i];

	++current_stun_server;

	return result;
}

address_and_port stun_server_provider::get_next_port_probe(const nat_port_probing_settings& settings) {
	auto result = settings.host;

	const auto num_available_without_first = settings.num_available - 1;
	const auto first_port_offset = 1;

	const auto current_port_i = first_port_offset + current_probed_port % num_available_without_first;
	result.default_port += current_port_i;

	++current_probed_port;

	return result;
}

double stun_server_provider::seconds_to_wait_for_next(const double usage_cooldown_secs) const {
	const auto current_server_i = current_stun_server % servers.size();
	const auto& ts = usage_timestamps[current_server_i];

	if (ts == -1) {
		return 0.0;
	}

	const auto passed = yojimbo_time() - ts;
	return usage_cooldown_secs - passed;
}