#include <optional>
#include "augs/network/port_type.h"
#include "augs/network/netcode_sockets.h"
#include "augs/network/network_types.h"
#include "augs/network/netcode_utils.h"

#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/gui/my_network_details_gui.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/masterserver/masterserver_requests.h"

double yojimbo_time();

constexpr auto address_resolution_interval = 0.5;

void my_network_details_gui_state::handle_response(const netcode_address_t& resolution_host, const netcode_address_t& resolved_addr) {
	auto check_resolved = [&](auto& into) {
		if (into.destination == resolution_host)
		{
			into.resolved = resolved_addr;
		}
	};

	check_resolved(info.first);
	check_resolved(info.second);
}

void my_network_details_gui_state::reset() {
	info = {};
}

void my_network_details_gui_state::advance_address_resolution(netcode_socket_t& udp_socket, const netcode_address_t& resolution_host, const port_type extra_port) {
	auto advance_resolution_of = [&](auto& info, const std::optional<port_type> alternate_port = std::nullopt) {
		if (info.completed()) {
			return;
		}

		if (try_fire_interval(address_resolution_interval, info.when_last)) {
			auto serv_addr = resolution_host;

			if (alternate_port != std::nullopt) {
				serv_addr.port = *alternate_port;
			}

			info.destination = serv_addr;

			::tell_me_my_address(udp_socket, serv_addr);
		}
	};

	advance_resolution_of(info.first);
	advance_resolution_of(info.second, extra_port);
}

void my_network_details_gui_state::perform() {
	using namespace augs::imgui;

	auto window = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!window) {
		return;
	}

	text_disabled("Tuples: Destination address:NAT mapped address");

	auto show_line = [&](const auto& info) {
		if (info.when_last == 0) {
			return;
		}

		auto dest_address = ::ToString(info.destination);
		input_text("###Destination", dest_address, ImGuiInputTextFlags_ReadOnly);

		ImGui::SameLine();

		auto nat_mapped_address = info.resolved ? ::ToString(*info.resolved) : std::string("Resolving...");
		input_text("###NatMapped", nat_mapped_address, ImGuiInputTextFlags_ReadOnly);
	};

	show_line(info.first);
	auto id = scoped_id(1);
	show_line(info.second);
}
