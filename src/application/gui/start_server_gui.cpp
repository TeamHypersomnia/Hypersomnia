#include <cstdint>
#include "application/gui/start_server_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/maybe_different.h"
#include "augs/misc/imgui/imgui_enum_radio.h"
#include "application/gui/do_server_vars.h"
#include "application/setups/server/server_vars.h"
#include "application/nat/nat_detection_session.h"
#include "augs/misc/imgui/imgui_enum_combo.h"
#include "augs/misc/date_time.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

std::string censor_ips(std::string text);

bool perform_arena_chooser(
	arena_identifier& current_arena,
	const server_runtime_info* info = nullptr
);

bool perform_game_mode_chooser(game_mode_name_type& current_arena, const std::string& caption = "Game mode");

server_temp_var_overrides start_server_gui_state::get_initial_overrides() const {
	auto result = server_temp_var_overrides();

	if (!enable_bots) {
		result.bots = { {}, 0u, 0u };
	}
	else {
		result.bots = { {}, uint8_t(bot_allies), uint8_t(bot_enemies) };
	}

	result.bot_difficulty.difficulty = bot_difficulty;

	result.bots.requester = mode_player_id::machine_admin();
	result.bot_difficulty.requester = mode_player_id::machine_admin();

	return result;
}

bool start_server_gui_state::perform(
	server_listen_input& into,
	server_vars& into_vars,

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
As a host, you will experience no lag whatsoever - your experience will be flawless.

This option is perfect for quick matches between friends and duels of honor.

)";

		text_color(help_1, tutorial_col);

		ImGui::Separator();

		text_color("Dedicated server instance:", orange);

		const auto help_2 = R"(
Fully featured, separate server process designed to run 24/7.
It will be running even after exiting the game.
You can shut it down only through a RCON command accessible to authorized clients.
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

#if BUILD_NATIVE_SOCKETS
	if (show_nat_details && nat_detection != nullptr) {
		auto result = augs::imgui::cond_scoped_window(show_nat_details, "NAT detection details", &show_nat_details, ImGuiWindowFlags_AlwaysAutoResize);

		const auto log_text = nat_detection->get_full_log();

		if (ImGui::Button("Copy to clipboard")) {
			ImGui::SetClipboardText(log_text.c_str());
		}

		const auto log_color = rgba(210, 210, 210, 255);
		text_color(::censor_ips(log_text), log_color);
	}
#else
	(void)nat_detection;
#endif

	{
		auto child = scoped_child("host view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		// auto& scope_cfg = into;

		input_text_with_hint("Server name", "(default)", into_vars.server_name);

		if (perform_arena_chooser(into_vars.arena)) {
			into_vars.game_mode = "";
		}

		perform_game_mode_chooser(into_vars.game_mode);

		slider("Slots", into.slots, 2, 64);

		{
			const auto wave_time = augs::steady_secs() / 4.0;
			const auto wave_color = rgba::get_bright_wave(wave_time, 0.55);
			auto scope = scoped_text_color(wave_color);
			checkbox("Bots (NEW!)", enable_bots);
		}

		if (enable_bots) {
			auto ind = scoped_indent();

			enum_combo("Difficulty", bot_difficulty);

			text_disabled("Sending \"/bots hard\" in chat changes difficulty.");

			slider("Num allies", bot_allies, 0, 20);
			slider("Num enemies", bot_enemies, 0, 20);

			if (bot_allies + bot_enemies > 20) {
				auto dt = bot_allies + bot_enemies - 20;

				if (bot_allies < bot_enemies) {
					bot_allies -= dt;
				}
				else {
					bot_enemies -= dt;
				}
			}

			bot_allies = std::max(bot_allies, 0);
			bot_enemies = std::max(bot_enemies, 0);

			text_disabled("Sending \"/bots 2 5\" in chat changes bot quotas.");
		}

		// input_text<100>("Address (IPv4 or IPv6)", into.ip);
		// text_disabled("Tip: the address can be either IPv4 or IPv6.\nFor example, you can put the IPv6 loopback address, which is \"::1\".");

#if PLATFORM_WEB
		(void)currently_bound_port;
#else
		auto do_port = [&](const auto& label, auto& val) {
			auto chosen_port = static_cast<int>(val);

			ImGui::InputInt(label, std::addressof(chosen_port), 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
			val = static_cast<unsigned short>(std::clamp(chosen_port, 0, 65535));
		};

		{
			const bool has_custom_port = into.port != 0;
			bool ticked = has_custom_port;

			if (checkbox("Use custom port", ticked)) {
				into.port = ticked ? previous_chosen_port : 0;
			}
		}

		const bool has_custom_port = into.port != 0;

		if (has_custom_port) {
			auto ind = scoped_indent();

			previous_chosen_port = into.port;
			do_port("##Port", into.port);
		}

		const bool port_bound_successfully = into.port == currently_bound_port;

		if (has_custom_port) {
			text("Currently bound port:");

			ImGui::SameLine();

			text_color(std::to_string(currently_bound_port), port_bound_successfully ? green : orange);

			if (!port_bound_successfully) {
				ImGui::SameLine();
				text_color(typesafe_sprintf("(Could not bind to %x!)", into.port), orange);
			}
		}
		else {
			text("Randomly selected port:");

			ImGui::SameLine();

			text_color(std::to_string(currently_bound_port), green);
		}

		checkbox("I'm behind router", into_vars.is_behind_nat);

		if (ImGui::IsItemHovered()) {
			text_tooltip("If you're behind a router, leave this on.\n\nThis technique lets your friends establish a connection \nwithout port forwarding necessary, possibly using relay servers.\n\nYou can disable this if you plan to only play over LAN.");
		}

		if (into_vars.is_behind_nat) {
			if (nat_detection == nullptr) {
				text_color("NAT detection and traversal was disabled for this launch.", red);
			}
			else {
				if (const auto result = nat_detection->query_result()) {
					const auto color = nat_type_to_color(result->type);

					text_color(result->describe(), color);

					ImGui::SameLine();

					if (ImGui::Button("Details")) {
						show_nat_details = true;
					}

					if (color == green) {
						text_color("No port forwarding necessary.", color);
					}
					else {
						text_color("Relay servers might be used.\nConsider port forwarding.", color);
					}
				}
				else {
					text_color(typesafe_sprintf("NAT detection for port %x in progress...", currently_bound_port), red);

					ImGui::SameLine();

					if (ImGui::Button("Details")) {
						show_nat_details = true;
					}
				}
			}
		}

#if 0
#if IS_PRODUCTION_BUILD
		const bool choose_instance = !is_steam_client;
#else
		const bool choose_instance = true;
#endif

		if (choose_instance) {
			/* 
				Don't show this on the Steam client because two Steam processes could mess something up.
				CLI tools actually launch without Steam API but they read from the "user" folder so the user might be surprised.

				For running dedicated servers you're better off downloading a non-steam version of the game.
			*/

			ImGui::Separator();
			text("Server instance type:");

			ImGui::SameLine();

			if (ImGui::Button ("?")) {
				show_help = true;
			}

			enum_radio(type, true);
			ImGui::Separator();
		}

#endif
#endif

		text_disabled("See Settings->Server for more options to tweak.\n\n");

		//text_disabled("Tip: to quickly host a server, you can press Shift+H here or in the main menu,\ninstead of clicking \"Launch!\" with your mouse.");
	}

	{
		auto scope = scoped_child("launch cancel");

		ImGui::Separator();

		{
			const bool valid = into_vars.server_name.empty() || is_nickname_valid_characters(into_vars.server_name);

			auto scope = maybe_disabled_cols({}, !valid);

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

