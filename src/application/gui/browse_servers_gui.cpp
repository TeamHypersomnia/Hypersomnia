#include <future>

#include "application/gui/browse_servers_gui.h"
#include "augs/network/netcode_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "3rdparty/cpp-httplib/httplib.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/log.h"

browse_servers_gui_state::~browse_servers_gui_state() = default;

struct browse_servers_gui_internal {
	std::optional<httplib::Client> http;
	std::future<std::shared_ptr<httplib::Response>> future_response;
};


browse_servers_gui_state::browse_servers_gui_state(const std::string& title) 
	: base(title), data(std::make_unique<browse_servers_gui_internal>()) 
{

}

#if 0
static std::string get_internal_ip() {
	const char* google_dns_server = "8.8.8.8";
	int dns_port = 53;

	struct sockaddr_in serv;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
	{
		return "";
	}

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(google_dns_server);
	serv.sin_port = htons(dns_port);

	int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));

	if (err < 0)
	{
		return "";
	}

	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (struct sockaddr*)&name, &namelen);

	char buffer[80];
	const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);

	if (p != NULL)
	{
		return buffer;
	}

	close(sock);
	return "";
}
#endif

double yojimbo_time();

void browse_servers_gui_state::refresh_server_list(const browse_servers_input in) {
	using namespace httplib;

	error_message.clear();
	server_list.clear();
	selected_server.reset();

	if (data->future_response.valid()) {
		return;
	}

	auto& http_opt = data->http;

	data->future_response = launch_async(
		[&http_opt, host_url = in.server_list_provider]() {
			const auto timeout = 5;
			http_opt.emplace(host_url.c_str(), 8080, timeout);
			LOG("Refreshing server list from %x.", host_url);

			auto progress = [](uint64_t len, uint64_t total) {
				(void)len;
				(void)total;

				return true;
			};

			auto& http = *http_opt;
			http.follow_location(true);

			return http.Get("/server_list_binary", progress);
		}
	);

	when_last_downloaded_server_list = yojimbo_time();
}

void browse_servers_gui_state::show_server_list() {
	using namespace augs::imgui;

	for (const auto& sp : filtered_server_list) {
		const auto& s = *sp;
		const auto& d = s.data;

		if (ImGui::Selectable(d.server_name.c_str(), selected_server == s.address, ImGuiSelectableFlags_SpanAllColumns)) {
			selected_server = s.address;
		}

		if (ImGui::IsItemClicked()) {
			if (ImGui::IsMouseDoubleClicked(0)) {
				LOG("Selected server list entry: %x (%x)", ToString(s.address), d.server_name);
			}
		}

		ImGui::NextColumn();

		if (d.game_mode.is_set()) {
			const auto game_mode_name = d.game_mode.dispatch([](auto d) {
				using D = decltype(d);
				return format_field_name(get_type_name<D>());
			});

			text(game_mode_name);
		}
		else {
			text("<unknown>");
		}

		ImGui::NextColumn();


		const auto max_players = std::min(d.max_fighting, d.max_online);
		text(typesafe_sprintf("%x/%x", d.num_fighting, max_players));

		ImGui::NextColumn();

		const auto num_spectators = d.num_online - d.num_fighting;
		const auto max_spectators = d.max_online - d.num_fighting;

		text(typesafe_sprintf("%x/%x", num_spectators, max_spectators));

		ImGui::NextColumn();
		text(d.current_arena);

		ImGui::NextColumn();

		const auto ping = s.ping;

		if (ping != -1) {
			if (ping > 999) {
				text("999>");
			}
			else {
				text(typesafe_sprintf("%x", ping));
			}
		}

		ImGui::NextColumn();
	}
}

bool successful(const int http_status_code);

void browse_servers_gui_state::perform(const browse_servers_input in) {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	centered_size_mult = vec2(0.75f, 0.6f);

	auto imgui_window = make_scoped_window();

	if (!imgui_window) {
		return;
	}

	const auto couldnt_download = std::string("Couldn't download the server list.\n");

	auto handle_response = [&](auto response) {
		if (response == nullptr) {
			error_message = "Could not establish connection with the server list host.";
			return;
		}

		const auto status = response->status;

		LOG("Server list response status: %x", status);

		if (!successful(status)) {
			error_message = couldnt_download + "HTTP response: " + std::to_string(status);
			return;
		}

		const auto& bytes = response->body;

		LOG("Server list response bytes: %x", bytes.size());

		const auto buffer_ptr = augs::cpointer_to_buffer{ reinterpret_cast<const std::byte*>(bytes.data()), bytes.size() };

		auto ss = augs::cptr_memory_stream{ buffer_ptr };

		try {
			while (ss.get_unread_bytes() > 0) {
				server_list_entry entry;

				augs::read_bytes(ss, entry.address);
				augs::read_bytes(ss, entry.data);

				server_list.emplace_back(std::move(entry));
			}
		}
		catch (const augs::stream_read_error& err) {
			error_message = "There was a problem deserializing the server list:\n" + std::string(err.what()) + "\n\nTry restarting the game and updating your client!";
			server_list.clear();
		}
	};

	if (valid_and_is_ready(data->future_response)) {
		handle_response(data->future_response.get());
	}

	thread_local ImGuiTextFilter filter;

	filter.Draw();
	filtered_server_list.clear();

	for (auto& s : server_list) {
		if (show_only_responding_servers) {
			if (s.ping == -1) {
				continue;
			}
		}

		if (filter.PassFilter(s.data.server_name.c_str())) {
			filtered_server_list.push_back(&s);
		}
	}

	ImGui::Separator();

	if (when_last_downloaded_server_list == 0) {
		refresh_server_list(in);
	}

	{
		auto child = scoped_child("list view", ImVec2(0, 2 * -(ImGui::GetFrameHeightWithSpacing() + 4)));


		const auto num_filtered_results = filtered_server_list.size();
		const auto servers_label = typesafe_sprintf("Servers (%x)", num_filtered_results);

		ImGui::Columns(6);
		ImGui::SetColumnWidth(0, ImGui::CalcTextSize("9").x * 80);
		ImGui::SetColumnWidth(1, ImGui::CalcTextSize("9").x * 30);
		ImGui::SetColumnWidth(2, ImGui::CalcTextSize("Players  ").x);
		ImGui::SetColumnWidth(3, ImGui::CalcTextSize("Spectators  ").x);
		ImGui::SetColumnWidth(4, ImGui::CalcTextSize("9").x * (max_arena_name_length_v + 1));
		ImGui::SetColumnWidth(5, ImGui::CalcTextSize("999999").x);

		text_disabled(servers_label.c_str());
		ImGui::NextColumn();
		text_disabled("Game mode");
		ImGui::NextColumn();
		text_disabled("Players");
		ImGui::NextColumn();
		text_disabled("Spectators");
		ImGui::NextColumn();
		text_disabled("Arena");
		ImGui::NextColumn();
		text_disabled("Ping");
		ImGui::NextColumn();

		ImGui::Separator();

		if (error_message.size() > 0) {
			text_color(error_message, red);
		}
		else {
			if (num_filtered_results == 0 && server_list.size() > 0) {
				text_disabled("No servers match applied filters.");
			}

			if (server_list.empty()) {
				if (data->future_response.valid()) {
					text_disabled("Downloading the server list...");
				}
				else {
					text_disabled("No servers are currently online.");
				}
			}
			else {
				show_server_list();
			}
		}
	}

	{
		auto child = scoped_child("buttons view");

		ImGui::Separator();

		checkbox("Only responding", show_only_responding_servers);

		if (ImGui::Button("Refresh list")) {
			refresh_server_list(in);
		}
	}
}
