#pragma once
#include "application/gui/do_server_vars.h"
#include "application/gui/client/rcon_gui.h"
#include "application/gui/pretty_tabs.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"

template <class F>
void do_pending_rcon_payloads(
	rcon_gui_state& state, 
	F&& on_new_payload
) {
	auto& custom_commands = state.custom_commands_text;

	if (state.request_execute_custom_game_commands) {
		on_new_payload(custom_commands);
		state.request_execute_custom_game_commands = false;
	}
}

template <class F>
void perform_rcon_gui(
	rcon_gui_state& state, 
	const bool is_remote_server,
	const bool during_ranked,
	F&& on_new_payload
) {
	const server_runtime_info* runtime_info = nullptr;

	if (is_remote_server) {
		runtime_info = &state.runtime_info;
	}

	(void)during_ranked;
	using namespace augs::imgui;

	const bool has_maintenance_tab = is_remote_server;

	const auto window_name = is_remote_server ? "Server Control (RCON)" : "Server Control";

	center_next_window(vec2::square(0.55f), ImGuiCond_Once);

	auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar);
	centered_text(window_name);

	const auto level = state.level;
	auto& custom_commands = state.custom_commands_text;

	if (level == rcon_level_type::INTEGRATED_ONLY) {
		text_color("This is an integrated server. Only the host has RCON access.", red);
		ImGui::Separator();
	}
	else if (level == rcon_level_type::DENIED) {
		text_color("Access denied.", red);
		ImGui::Separator();
	}
#if IS_PRODUCTION_BUILD
	else if (during_ranked) {
		text_color("RCON is unavailable during ranked matches.", red);
		ImGui::Separator();
	}
#endif
	else {
		do_pretty_tabs(state.active_pane);

		using RS = server_maintenance_command;

		auto do_command_button = [&](const std::string& label, const auto cmd) {
			if (ImGui::Button(label.c_str())) {
				on_new_payload(cmd);
				return true;
			}

			return false;
		};

		auto do_server_vars_panel = [&](
			auto& edited,
			auto& last_saved,
			auto& applying_flag,
			const auto pane
		) { 
			{
				auto child = scoped_child("settings view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
				auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);


				do_server_vars(
					edited,
					last_saved,
					pane,
					runtime_info
				);
			}

			{
				auto scope = scoped_child("save revert");

				ImGui::Separator();

				if (applying_flag) {
					text_color("Applying changes...", yellow);
				}
				else if (last_saved != edited) {
					if (ImGui::Button("Apply & Save")) {
						applying_flag = true;

						on_new_payload(edited);
					}

					ImGui::SameLine();

					if (ImGui::Button("Revert all")) {
						edited = last_saved;
					}
				}
			}
		};

		switch (state.active_pane) {
			case rcon_pane::ARENAS:
			case rcon_pane::VARS:
				do_server_vars_panel(
					state.edited_sv_vars, 
					state.last_applied_sv_vars, 
					state.applying_sv_vars,
					state.active_pane
				);

				break;

			case rcon_pane::MATCH:
				using MC = match_command;

				augs::for_each_enum_except_bounds([&](const MC cmd) {
					do_command_button(format_enum(cmd), cmd);
				});

				ImGui::Separator();

				text_color("Execute custom game commands", yellow);

				input_multiline_text("##CommandsArea", custom_commands, 10);

				{
					const auto len = custom_commands.length();
					const bool len_valid = len > 0 && len <= default_max_std_string_length_v;
						
					auto scope = maybe_disabled_cols({}, !len_valid);

					if (ImGui::Button("Execute") || state.request_execute_custom_game_commands) {
						on_new_payload(custom_commands);
						state.request_execute_custom_game_commands = false;
					}
				}

				break;

			case rcon_pane::MAINTENANCE:
				{
					if (has_maintenance_tab) {
						if (do_command_button("Refresh map list", RS::REQUEST_RUNTIME_INFO)) {
							LOG("Requesting the server refresh the map list.");
						}

						auto disabled = maybe_disabled_cols({}, level == rcon_level_type::BASIC);

						if (level == rcon_level_type::BASIC) {
							text_disabled("Your RCON level is too low to perform these tasks.");
						}

						if (do_command_button("Shutdown server", RS::SHUTDOWN)) {
							LOG("Requesting the server to shut down");
						}

						if (do_command_button("Restart server", RS::RESTART)) {
							LOG("Requesting the server to restart");
						}

						if (do_command_button("Check and apply updates now", RS::CHECK_FOR_UPDATES_NOW)) {
							LOG("Requesting the server to restart");
						}

						do_command_button("Download logs", RS::DOWNLOAD_LOGS); 

						if (runtime_info) {
							ImGui::Separator();
							text_color("Changed RCON settings", yellow);
							ImGui::Separator();

							auto prefs = runtime_info->runtime_prefs;
							input_multiline_text("##RconJson", prefs, 10, ImGuiInputTextFlags_ReadOnly);

							do_command_button("Clear all", RS::CLEAR_RUNTIME_PREFS); 
						}
					}
					else {
						text_color("Nothing to maintain on an integrated server!", orange);
					}
				}

				break;

			default: break;
		}
	}

	ImGui::Separator();
}
