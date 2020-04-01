#include "application/gui/start_server_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "application/setups/editor/detail/maybe_different_colors.h"
#include "augs/misc/imgui/imgui_enum_radio.h"
#include "application/gui/do_server_vars.h"
#include "application/setups/server/server_vars.h"
#include "application/nat/nat_detection_session.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

void perform_arena_chooser(arena_identifier& current_arena);

bool start_server_gui_state::perform(
	server_start_input& into,
	server_vars& into_vars,
	server_solvable_vars& into_solvable_vars,

	const nat_detection_session* nat_detection,
	const port_type currently_bound_port
) {
	// int field_id = 0;

	if (!show) {
		return false;
	}

	using namespace augs::imgui;

	bool result = false;

	centered_size_mult = vec2::square(0.45f);

	auto window = make_scoped_window();

	if (!window) {
		return false;
	}

	if (show_help) {
		const auto tutorial_col = rgba(210, 210, 210, 255);

		auto result = augs::imgui::cond_scoped_window(show_help, "Help", &show_help, ImGuiWindowFlags_AlwaysAutoResize);

		text_color("Integrated server instance:", yellow);

		const auto help_1 = R"(
Simplified server running in the game's process.
The server will be shut down as soon as you exit the game.
As a host, you will experience no lag whatsoever - your experience will be seamless.

This option is perfect for quick matches between friends and duels of honor.

)";

		text_color(help_1, tutorial_col);

		ImGui::Separator();

		text_color("Dedicated server instance:", orange);

		const auto help_2 = R"(
Fully featured, separate server process designed to run 24/7.
It will be running even after exiting the game.
You can shut it down only through a RCON command accessible to authorized clients.
To play, you will need to manually connect to localhost after launching the server. 
You will experience a minimal lag (on the order of 10ms). 
For balance, you can increase the lag via Settings->Client->Enable lag simulator.

This option is perfect for hosting a persistent server on your machine.

)";

		text_color(help_2, tutorial_col);

		ImGui::Separator();

		const auto help_3 = R"(
Unless lag simulation is enabled, the host has an immense advantage over the other players,
especially if the host is running an integrated server instance.

Next time, consider letting your friend be the one to host the server,
so that they can experience seamless gameplay too,
as well as to test your skills in a laggy environment.

)";

		text_color(help_3, tutorial_col);

		if (ImGui::Button("Close")) {
			show_help = false;
		}
	}

	if (show_nat_details && nat_detection != nullptr) {
		auto result = augs::imgui::cond_scoped_window(show_nat_details, "NAT detection details", &show_nat_details, ImGuiWindowFlags_AlwaysAutoResize);

		const auto log_text = nat_detection->get_full_log();

		if (ImGui::Button("Copy to clipboard")) {
			ImGui::SetClipboardText(log_text.c_str());
		}

		const auto log_color = rgba(210, 210, 210, 255);
		text_color(log_text, log_color);
	}

#if NDEBUG && PLATFORM_UNIX
	const bool is_dedicated = instance_type == server_instance_type::DEDICATED;
	const bool please_use_cmdline = is_dedicated;
#else
	const bool please_use_cmdline = false;
#endif

	const bool is_disabled = please_use_cmdline;

	{
		auto child = scoped_child("host view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		// auto& scope_cfg = into;

		perform_arena_chooser(into_solvable_vars.current_arena);
		input_text("Server name", into_vars.server_name);

		input_text<100>("Address (IPv4 or IPv6)", into.ip);
		// text_disabled("Tip: the address can be either IPv4 or IPv6.\nFor example, you can put the IPv6 loopback address, which is \"::1\".");

		auto do_port = [&](const auto& label, auto& val) {
			auto chosen_port = static_cast<int>(val);

			ImGui::InputInt(label, std::addressof(chosen_port), 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
			val = static_cast<unsigned short>(std::clamp(chosen_port, 1024, 65535));
		};

		do_port("Port", into.port);

		const bool port_bound_successfully = into.port == currently_bound_port;

		text("Currently bound port:");
		ImGui::SameLine();

		text_color(std::to_string(currently_bound_port), port_bound_successfully ? green : orange);

		if (!port_bound_successfully) {
			ImGui::SameLine();
			text_color("(Could not bind to %x!)", orange);
		}

		if (nat_detection == nullptr) {
			text_color("NAT detection and traversal was disabled for this launch.", red);
		}
		else {
			if (const auto result = nat_detection->query_result()) {
				const rgba nat_colors[5] = { green, green, yellow, yellow, orange };
				const auto color_index = static_cast<int>(result->type);

				text_color(result->describe(), nat_colors[color_index]);
			}
			else {
				text_color(typesafe_sprintf("NAT detection for port %x in progress...", currently_bound_port), red);
			}

			ImGui::SameLine();

			if (ImGui::Button("Details")) {
				show_nat_details = true;
			}
		}

		{
			ImGui::Separator();
			text("Server instance type:");

			ImGui::SameLine();

			if (ImGui::Button ("?")) {
				show_help = true;
			}

			enum_radio(instance_type, true);
			ImGui::Separator();
		}

		if (is_disabled) {
			text_color("On Linux, please use the command line to spawn a dedicated server.", red);
		}
		else {
			if (instance_type == server_instance_type::INTEGRATED) {
				slider("Max incoming connections", into.max_connections, 1, 64);
				text_disabled("Tip: this number does not include the integrated, local player on the server.\nIf you want to play a 1v1 with someone and not allow anyone else to join or watch,\nyou want to set this value to 1.\n\n");
			}
			else {
				slider("Max incoming connections", into.max_connections, 2, 64);
			}

			text_disabled("See Settings->Server for more options to tweak.\n\n");

			text_disabled("Tip: to quickly host a server, you can press Shift+H here or in the main menu,\ninstead of clicking \"Launch!\" with your mouse.");
		}
	}

	{
		auto scope = scoped_child("launch cancel");

		ImGui::Separator();

		{
			auto scope = maybe_disabled_cols({}, is_disabled);

			if (ImGui::Button("Launch!")) {
				result = true;
				//show = false;
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			show = false;
		}
	}

	return result;
}

