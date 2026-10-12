#include <thread>
#include <future>
#include "application/gui/start_client_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

#include "augs/network/network_types.h"
#include "augs/misc/imgui/maybe_different.h"
#include "augs/templates/thread_templates.h"
#include "augs/window_framework/window.h"

#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/filesystem/file.h"

#include "augs/misc/imgui/imgui_enum_radio.h"
#include "application/gui/demo_chooser.h"
#include "application/setups/client/demo_file.h"
#include "application/gui/pretty_tabs.h"
#include "augs/readwrite/byte_readwrite.h"
#include "steam_integration.h"
#include "steam_integration_helpers.hpp"
#include "application/gui/server_list_entry.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

void start_client_gui_state::clear_demo_choice() {
	demo_choice_result = demo_choice_result_type::SHOULD_ANALYZE;
	demo_path.clear();
	demo_meta = {};
	demo_size = {};
}

bool start_client_gui_state::perform(
	const std::optional<server_list_entry>& best_server,
	const bool refresh_in_progress,
	augs::window& window,
	client_connect_string& into_connect_string,
	std::string& into_displayed_connecting_server_name,
	client_vars& into_vars,
	const bool streamer_mode
) {
	if (!show) {
		return false;
	}
	
	using namespace augs::imgui;

#if WEB_LOWEND
	centered_size_mult = vec2(0.55f, 0.7f);
#else
	centered_size_mult = vec2(0.55f, 0.4f);
#endif
	
	auto imgui_window = make_scoped_window();

	if (!imgui_window) {
		return false;
	}

	bool result = false;

	{
		auto child = scoped_child("connect view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		do_pretty_tabs(current_tab);

		// auto& scope_cfg = into;

		if (current_tab == start_client_tab_type::REPLAY_DEMO) {
			using D = demo_choice_result_type;

			thread_local demo_chooser chooser;

			// text_disabled(typesafe_sprintf("The demos are stored inside \"%x\" folder.", DEMOS_DIR));

			chooser.perform(
				"##replay_demo_chooser",
				demo_path.string(),
				augs::path_type(DEMOS_DIR),
				[&](const auto& new_choice) {
					demo_path = new_choice.path;
					demo_choice_result = D::SHOULD_ANALYZE;
				}
			);

#if !PLATFORM_WEB
			{
				auto scope = maybe_disabled_cols({}, demo_path.empty());

				if (ImGui::Button("Demo in explorer")) {
					window.reveal_in_explorer(demo_path);
				}

				ImGui::SameLine();
			}

			if (ImGui::Button("Demo folder in explorer")) {
				window.reveal_in_explorer(DEMOS_DIR);
			}
#endif

			if (!demo_path.empty() && demo_choice_result == D::SHOULD_ANALYZE) {
				try {
					auto t = augs::open_binary_input_stream(demo_path);

					augs::read_bytes(t, demo_meta);
					demo_size = readable_bytesize(augs::get_file_size(demo_path));

					if (demo_meta.version == hypersomnia_version()) {
						demo_choice_result = D::OK;
					}
					else {
						demo_choice_result = D::MIGHT_BE_INCOMPATIBLE;
					}
				}
				catch (const std::ifstream::failure& err) {
					demo_choice_result = D::FILE_OPEN_ERROR;
				}
				catch (const augs::stream_read_error& err) {
					demo_choice_result = D::FILE_OPEN_ERROR;
				}
			}

			switch (demo_choice_result) {
				case D::FILE_OPEN_ERROR:
					text_color(typesafe_sprintf("Could not open:\n%x\n\nWrong path or the file might be corrupt.", demo_path), red);
					break;
				case D::MIGHT_BE_INCOMPATIBLE:
					text_color(typesafe_sprintf("Your client differs from the one used to record this demo.\nYou may try replaying, but all bets are off - the game might even crash."), orange);
					break;
				case D::OK:
					text_color(typesafe_sprintf("OK! The demo's version matches your client."), green);
					break;
				default:
					break;
			}

			if (!demo_path.empty() && demo_choice_result != D::FILE_OPEN_ERROR) {
				text("Server address:");
				ImGui::SameLine();
				text_color(demo_meta.server_address, cyan);
				text("Size:");
				ImGui::SameLine();
				text_color(demo_size, cyan);
				text("\n");

				text("Version information:");

				text(demo_meta.version.get_summary());
			}
			else {
				text(typesafe_sprintf("Choose demo to replay (from %x).", DEMOS_DIR));
			}
		}
		else if (current_tab == start_client_tab_type::CUSTOM_ADDRESS) {
			base::acquire_keyboard_once();

			const auto flags = streamer_mode ? ImGuiInputTextFlags_Password : 0; 
			input_text<100>("Address (ipv4, ipv6, ipv4:port, hostname:port, [ipv6]:port)", custom_address, flags);
		}
		else if (current_tab == start_client_tab_type::BEST_SERVER) {
			if (!best_server.has_value()) {
				auto cs = scoped_style_color(ImGuiCol_FrameBg, rgba(255,255,255,0));

				auto sname = std::string("Finding the best server...");
				input_text<max_server_name_length_v>("##BestServer", sname, ImGuiInputTextFlags_ReadOnly);
			}
			else {
				{
					thread_local bool hovered = false;

					auto cs = std::make_tuple(
						cond_scoped_style_color(hovered, ImGuiCol_FrameBg, rgba(255,255,255,60)),
						cond_scoped_style_color(!hovered, ImGuiCol_FrameBg, rgba(255,255,255,20))
					);

					auto sname = std::string(best_server->heartbeat.server_name);
					//ImGui::Selectable( "huh t", false, 0, { ImGui::CalcItemWidth(), 0.0f });

					input_text<max_server_name_length_v>("##BestServer", sname, ImGuiInputTextFlags_ReadOnly);

					hovered = ImGui::IsItemHovered();

					if (hovered) {
						text_tooltip("Show in Server Browser");
					}

					if (ImGui::IsItemActivated()) {
						ImGui::ClearActiveID();
						request_server_list_open = true;
					}
				}	

			}

			ImGui::SameLine();

			{
				auto curs = scoped_preserve_cursor();

				text_color("Players:", green);
				ImGui::SameLine();
				text(best_server.has_value() ? std::to_string(best_server->heartbeat.num_online) : std::string("?"));

				ImGui::SameLine();

				text_color("Ping:", cyan);
				ImGui::SameLine();

				//const auto col = best_server->progress.get_ping_color();

#if PLATFORM_WEB
				text_disabled("?");
#else
				text(best_server.has_value() ? best_server->progress.get_ping_string() : std::string("?"));
#endif
			}

			text_color("Players: 99  Ping: 999", rgba(0,0,0,0));
			ImGui::SameLine();

			{
				auto scope = maybe_disabled_cols({}, refresh_in_progress);

				if (ImGui::Button("Refresh")) {
					request_refresh_best_server = true;
				}
			}

		}

		if (current_tab != start_client_tab_type::REPLAY_DEMO) {
			/*
				The nickname and the avatar are in Settings - Account.
			*/

			checkbox("Record demo", into_vars.record_demo);
			ImGui::SameLine();
			text_disabled("You can later rewatch your match in Replay tab.");

			if (const bool show_browser_tip = false) {
				if (current_tab == start_client_tab_type::CUSTOM_ADDRESS) {
					text_disabled("It is best to use the server browser to connect to custom servers.\nThe server browser allows you to connect to servers behind routers.");
				}
			}

			//text_disabled("Tip: to quickly connect, you can press Shift+C here or in the main menu,\ninstead of clicking \"Connect!\" with your mouse.");
		}
	}

	{
		auto scope = scoped_child("connect cancel");

		ImGui::Separator();

		if (current_tab == start_client_tab_type::REPLAY_DEMO) {
			using D = demo_choice_result_type;

			const bool result_good = demo_choice_result == D::OK || demo_choice_result == D::MIGHT_BE_INCOMPATIBLE;

			auto scope = maybe_disabled_cols({}, !result_good || demo_path.empty());

			if (ImGui::Button("Replay!")) {
				result = true;
				into_connect_string = typesafe_sprintf("%x%x", demo_address_preffix_v, demo_path);
				into_displayed_connecting_server_name = demo_path.filename().string();
				//show = false;
			}
		}
		else {
			const bool custom_addr_empty = 
				current_tab == start_client_tab_type::CUSTOM_ADDRESS
				&& custom_address.empty()
			;

			const bool best_still_unresolved = 
				current_tab == start_client_tab_type::BEST_SERVER
				&& best_server == std::nullopt
			;

			auto scope = maybe_disabled_cols({}, best_still_unresolved || custom_addr_empty || !is_nickname_valid_characters(into_vars.nickname) || !::nickname_len_in_range(into_vars.nickname.length()));

			if (ImGui::Button("Connect!")) {
				switch (current_tab) {
					case start_client_tab_type::BEST_SERVER:
						if (best_server) {
							into_connect_string = std::string(best_server->get_my_connect_string());
							into_displayed_connecting_server_name = best_server->heartbeat.server_name;
						}

						break;
					case start_client_tab_type::CUSTOM_ADDRESS:
						into_connect_string = custom_address;
						break;
					default:
						break;
				}

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

