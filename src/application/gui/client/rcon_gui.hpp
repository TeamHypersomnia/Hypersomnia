#pragma once
#include "application/gui/do_server_vars.h"
#include "application/gui/client/rcon_gui.h"
#include "application/gui/pretty_tabs.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"

template <class F>
void perform_rcon_gui(
	rcon_gui_state& state, 
	const bool is_remote_server,
	F&& on_new_payload
) {
	using namespace augs::imgui;

	const bool has_maintenance_tab = is_remote_server;

	const auto window_name = "Remote Control (RCON)";

	center_next_window(vec2::square(0.7f), ImGuiCond_Once);

	auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar);
	centered_text(window_name);

	const auto level = state.level;

	if (level == rcon_level_type::INTEGRATED_ONLY) {
		text_color("This is an integrated server. Only the host has RCON access.", red);
		ImGui::Separator();
	}
	else if (level == rcon_level_type::DENIED) {
		text_color("Access denied.", red);
		ImGui::Separator();
	}
	else {
		do_pretty_tabs(state.active_pane);

		using RS = rcon_commands::special;

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

				const server_runtime_info* runtime_info = nullptr;

				if (is_remote_server) {
					runtime_info = &state.runtime_info;
				}

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

				break;


			case rcon_pane::RULESETS:
				break;

			case rcon_pane::USERS:
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

						if (do_command_button("Restart & update server", RS::RESTART)) {
							LOG("Requesting the server to restart");
						}

						do_command_button("Download logs", RS::DOWNLOAD_LOGS); 
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
