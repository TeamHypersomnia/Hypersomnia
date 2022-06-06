#pragma once
#include "augs/log_path_getters.h"
#include "augs/filesystem/file.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"

std::string nat_traversal_state_to_string(const nat_traversal_session::state state) {
	using S = nat_traversal_session::state;

	switch (state) {
		case S::INIT:
			return "Initializing";
		case S::TRAVERSING:
			return "Traversing";
		case S::SERVER_STUN_REQUIREMENT_MET:
			return "Server STUN requirement met";
		case S::REQUESTING_REMOTE_PORT_INFO:
			return "Requesting remote port info";
		case S::TRAVERSAL_COMPLETE:
			return "Traversal complete";
		case S::TIMED_OUT:
			return "Traversal timed out";

		default:
			return "Unknown";
	}
}

rgba nat_traversal_state_to_color(const nat_traversal_session::state state) {
	using S = nat_traversal_session::state;

	switch (state) {
		case S::INIT:
			return orange;
		case S::SERVER_STUN_REQUIREMENT_MET:
			return cyan;
		case S::REQUESTING_REMOTE_PORT_INFO:
			return yellow;
		case S::TRAVERSING:
			return cyan;
		case S::TRAVERSAL_COMPLETE:
			return green;
		case S::TIMED_OUT:
			return red;

		default:
			return red;
	}
}


struct nat_traversal_details_window {
	bool rechoose_random_port_on_start = false;

	bool is_open = false;
	bool aborted = false;

	struct attempt_log {
		port_type bound_port;
		std::string log_text;
	};

	int current_attempt_index = -1;
	std::vector<attempt_log> attempts;
	
	void next_attempt() {
		++current_attempt_index;
		attempts.emplace_back();
	}

	void reset() {
		rechoose_random_port_on_start = true;

		attempts.clear();
		current_attempt_index = -1;
		aborted = false;
	}

	bool perform(
		augs::window& window,
		const port_type bound_local_port,
		const std::optional<nat_traversal_session>& session
	) {
		using namespace augs::imgui;

		const auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
		const auto title = "Traversing NAT...";

		bool result = false;

		const bool should_be_open = session != std::nullopt;

		if (should_be_open) {
			center_next_window(vec2(0.8f, 0.7f), ImGuiCond_Always);
		}

		if (auto popup = cond_scoped_modal_popup(should_be_open, title, nullptr, flags)) {
			{
				auto& current_attempt = attempts[current_attempt_index];

				if (!aborted) {
					const auto& log_text = session->get_full_log();

					current_attempt.log_text = log_text;
					current_attempt.bound_port = bound_local_port;
				}
			}

			const auto& shown_attempt = attempts[current_attempt_index];

			is_open = true;

			const auto client_type = session->input.client.type;
			const auto server_type = session->input.server.type;

			{
				auto child = scoped_child("details view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

				text_disabled("Bound port for this attempt:");
				ImGui::SameLine();
				text(std::to_string(shown_attempt.bound_port));

				text("Client NAT:");
				ImGui::SameLine();
				text_color(nat_type_to_string(client_type), nat_type_to_color(client_type));

				text("Server NAT:");
				ImGui::SameLine();
				text_color(nat_type_to_string(server_type), nat_type_to_color(server_type));

				text("Progress:");
				ImGui::SameLine();

				if (aborted) {
					text_color("Aborted", red);
				}
				else {
					const auto state = session->get_current_state();
					text_color(nat_traversal_state_to_string(state), nat_traversal_state_to_color(state));
				}

				{
					auto disabled = ::maybe_disabled_cols({}, !aborted);

					if (aborted) {
						auto width = scoped_item_width(200);
						text("Showing attempt");
						ImGui::SameLine();
						slider("##Showing attempt", current_attempt_index, 0, int(attempts.size()) - 1);
					}
					else {
						text("Attempt number %x", 1 + current_attempt_index);
					}
				}

				ImGui::Separator();

				const auto log_color = rgba(210, 210, 210, 255);
				text_color(shown_attempt.log_text, log_color);
			}

			{
				auto scope = scoped_child("abort");

				ImGui::Separator();

				if (ImGui::Button("Copy to clipboard")) {
					ImGui::SetClipboardText(shown_attempt.log_text.c_str());
				}

				ImGui::SameLine();

				if (ImGui::Button("Dump full log")) {
					auto full_log = std::string();

					for (std::size_t i = 0; i < attempts.size(); ++i) {
						const auto& a = attempts[i];

						full_log += typesafe_sprintf("\nPerforming traversal attempt no.: %x. Locally bound port: %x\n%x", i, a.bound_port, a.log_text);
					}

					/* delete first newline */
					full_log.erase(full_log.begin());

					const auto log_path = get_path_in_log_files("dumped_traversal_log.txt");
					augs::save_as_text(log_path, full_log);

					window.reveal_in_explorer(log_path);
				}

				ImGui::SameLine();

				{
					auto disabled = ::maybe_disabled_cols({}, aborted);

					if (ImGui::Button("Abort")) {
						aborted = true;
					}
				}

				{
					//auto disabled = ::maybe_disabled_cols({}, !aborted);

					ImGui::SameLine();

					if (ImGui::Button("Cancel")) {
						ImGui::CloseCurrentPopup();
						result = true;
					}
				}
			}
		}

		return result;
	}
};
