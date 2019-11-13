#pragma once
#include "application/gui/do_server_vars.h"
#include "application/gui/client/rcon_gui.h"
#include "application/gui/pretty_tabs.h"

template <class F>
void perform_rcon_gui(
	rcon_gui_state& state, 
	const bool has_maintenance_tab,
	F&& on_new_payload
) {
	using namespace augs::imgui;

	const auto window_name = "Remote Control (RCON)";

	ImGui::SetNextWindowPosCenter();

	ImGui::SetNextWindowSize((vec2(ImGui::GetIO().DisplaySize) * 0.7f).operator ImVec2(), ImGuiCond_Once);

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

		auto do_server_vars_panel = [&](
			auto& edited,
			auto& last_saved,
			auto& applying_flag
		) { 
			{
				auto child = scoped_child("settings view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
				auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

				do_server_vars(
					edited,
					last_saved
				);
			}

			{
				auto scope = scoped_child("save revert");

				ImGui::Separator();

				if (applying_flag) {
					text_color("Applying changes...", yellow);
				}
				else if (!augs::introspective_equal(last_saved, edited)) {
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

		auto do_command_button = [&](const std::string& label, const auto cmd) {
			if (ImGui::Button(label.c_str())) {
				on_new_payload(cmd);
				return true;
			}

			return false;
		};

		switch (state.active_pane) {
			case rcon_pane::ARENAS:
				do_server_vars_panel(
					state.edited_sv_solvable_vars,
					state.last_applied_sv_solvable_vars,
					state.applying_sv_solvable_vars
				);

				break;

			case rcon_pane::MATCH:
				using MC = match_command;

				augs::for_each_enum_except_bounds([&](const MC cmd) {
					do_command_button(format_enum(cmd), cmd);
				});

				break;

			case rcon_pane::VARS:
				do_server_vars_panel(
					state.edited_sv_vars, 
					state.last_applied_sv_vars, 
					state.applying_sv_vars
				);

				break;

			case rcon_pane::RULESETS:
				break;

			case rcon_pane::USERS:
				break;

			case rcon_pane::MAINTENANCE:
				if (has_maintenance_tab) {
					using RS = rcon_commands::special;

					if (do_command_button("Shutdown server", RS::SHUTDOWN)) {
						LOG("Requesting the server to shut down");
					}

					do_command_button("Download logs", RS::DOWNLOAD_LOGS); 
				}
				else {
					text_color("Nothing to maintain on an integrated server!", orange);
				}

				break;

			default: break;
		}
	}

	ImGui::Separator();
}
