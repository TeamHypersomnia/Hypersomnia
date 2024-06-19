#include "augs/templates/introspection_utils/introspective_equal.h"
#include "augs/templates/corresponding_field.h"

#include "augs/readwrite/json_readwrite.h"
#include "augs/window_framework/window.h"

#include "application/config_json_table.h"
#include "application/nat/stun_server_provider.h"

bool operator==(const ImVec2& a, const ImVec2& b) {
	return a.x == b.x && a.y == b.y;
}

bool operator==(const ImVec4& a, const ImVec4& b) {
	return rgba(a) == rgba(b);
}

bool operator==(const ImGuiStyle& a, const ImGuiStyle& b) {
	bool all_eq = true;

	augs::introspect(
		[&all_eq](const auto&, const auto& aa, const auto& bb) {
			if (!(aa == bb)) {
				all_eq = false;
			}
		},
		a,
		b
	);

	return all_eq;
}

namespace augs {
	double steady_secs();
}

config_json_table::config_json_table(const augs::path_type& config_lua_path) {
	load_patch(config_lua_path);
}

void config_json_table::load_patch(const augs::path_type& config_lua_path) {
	try {
		constexpr auto rapidjson_flags = 
			rapidjson::kParseDefaultFlags |
			rapidjson::kParseCommentsFlag | 
			rapidjson::kParseTrailingCommasFlag
		;

		auto doc = augs::json_document_from<rapidjson_flags>(augs::file_to_string(config_lua_path));
		augs::read_json(doc, *this);
	}
	catch (const augs::json_deserialization_error& err) {
		throw config_read_error(config_lua_path, err.what());
	}
	catch (const augs::file_open_error& err) {
		throw config_read_error(config_lua_path, err.what());
	}
}

void config_json_table::save_patch(const config_json_table& source, const augs::path_type& target_path) const {
	augs::save_as_json_diff(target_path, *this, source);
}

activity_type config_json_table::get_last_activity() const {
	return last_activity;
}

input_recording_type config_json_table::get_input_recording_mode() const {
	return debug.input_recording_mode;
}

stun_server_provider::stun_server_provider(const augs::path_type& list_file) {
	load(list_file);
}

namespace augs {
	std::vector<std::string> file_to_lines(const augs::path_type& path) {
		auto content = file_to_string(path);
		auto s = std::stringstream(content);

		std::vector<std::string> out;

		for (std::string line; std::getline(s, line); ) {
			out.emplace_back(line);
		}

		return out;
	}

	void lines_to_file(const path_type& path, const std::vector<std::string>& lines) {
		std::string total_text;

		for (auto& l : lines) {
			total_text += l + "\n";
		}	

		if (const bool cut_trailing_newline = total_text.size() > 0) {
			total_text.pop_back();
		}

		save_as_text(path, total_text);
	}
}

void stun_server_provider::load(const augs::path_type& list_file) {
	servers = augs::file_to_lines(list_file);
	usage_timestamps.resize(servers.size(), net_time_t(-1));
}

host_with_default_port stun_server_provider::get_next() {
	if (servers.empty()) {
		return {};
	}

	auto result = host_with_default_port();
	result.default_port = 3478;

	const auto current_server_i = current_stun_server % servers.size();

	usage_timestamps[current_server_i] = augs::steady_secs();
	result.address = servers[current_server_i];

	++current_stun_server;

	return result;
}

host_with_default_port stun_server_provider::get_next_port_probe(const nat_port_probing_settings& settings) {
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

	const auto passed = augs::steady_secs() - ts;
	return usage_cooldown_secs - passed;
}