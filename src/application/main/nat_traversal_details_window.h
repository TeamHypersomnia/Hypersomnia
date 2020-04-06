#pragma once

inline std::string nat_traversal_state_to_string(const nat_traversal_session::state state) {
	using S = nat_traversal_session::state;

	switch (state) {
		case S::INIT:
			return "Initializing";
		case S::TRAVERSING:
			return "Traversing";
		case S::AWAITING_STUN_RESPONSE:
			return "Awaiting STUN response";
		case S::TRAVERSAL_COMPLETE:
			return "Traversal complete";
		case S::TIMED_OUT:
			return "Traversal timed out";

		default:
			return "Unknown";
	}
}

inline rgba nat_traversal_state_to_color(const nat_traversal_session::state state) {
	using S = nat_traversal_session::state;

	switch (state) {
		case S::INIT:
			return orange;
		case S::AWAITING_STUN_RESPONSE:
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
	bool is_open = false;

	bool perform(
		const port_type bound_local_port,
		const std::optional<nat_traversal_session>& session
	) {
		using namespace augs::imgui;

		if (session == std::nullopt) {
			if (is_open) {
				ImGui::CloseCurrentPopup();
				is_open = false;
			}

			return false;
		}

		const auto& log_text = session->get_full_log();
		const auto title = "Traversing NAT...";

		center_next_window(vec2(0.55f, 0.4f), ImGuiCond_Always);

		const auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

		bool op = true;

		if (auto popup = scoped_modal_popup(title, &op, flags)) {
			is_open = true;

			const auto client_type = session->input.client.type;
			const auto server_type = session->input.server.type;

			{
				auto child = scoped_child("details view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

				text_disabled("Currently bound port:");
				ImGui::SameLine();
				text(std::to_string(bound_local_port));

				text("Client NAT:");
				ImGui::SameLine();
				text_color(nat_type_to_string(client_type), nat_type_to_color(client_type));

				text("Server NAT:");
				ImGui::SameLine();
				text_color(nat_type_to_string(server_type), nat_type_to_color(server_type));

				text("Progress:");
				ImGui::SameLine();

				const auto state = session->get_current_state();
				text_color(nat_traversal_state_to_string(state), nat_traversal_state_to_color(state));

				ImGui::Separator();

				const auto log_color = rgba(210, 210, 210, 255);
				text_color(log_text, log_color);
			}

			bool result = false;

			{
				auto scope = scoped_child("abort");

				ImGui::Separator();

				if (ImGui::Button("Copy to clipboard")) {
					ImGui::SetClipboardText(log_text.c_str());
				}

				ImGui::SameLine();

				if (ImGui::Button("Abort")) {
					result = true;
				}
			}

			return result;
		}

		return !op;
	}
};
