#pragma once

/*
	A server not on the server list is added by its line - "<address>,<name>,0x<key>", what its admin publishes
	(--print-join-string). Copied anywhere, it's taken as the game comes back into focus in the main menu (and once at the
	start): the server goes into the favorites, the browser opens on Favorites with it selected, a modal says so.

	A clipboard is looked at once - the same line again (e.g. the favorite removed since) is left alone. One already among
	the favorites: just shown, without the modal - but not at the start, when the line may have stayed there for days.
*/

bool browse_servers_gui_state::clipboard_worth_reading(const uint32_t times_activated) {
	const bool activated = times_activated != std::exchange(clipboard_activations, times_activated);
	clipboard_at_start = !std::exchange(clipboard_checked_once, true);
	return activated || clipboard_at_start;
}

bool browse_servers_gui_state::take_favorite_from_clipboard(const std::string& clipboard, favorite_servers& favorites) {
	if (clipboard == std::exchange(last_clipboard_seen, clipboard)) {
		return false;
	}

	const auto found = connect_target::find_server_identifiers(clipboard);

	if (!found.has_value() || !::is_pasted_address(found->address)) {
		return false;
	}

	const auto address = connect_target::without_default_port(found->address);
	const auto name = found->name.substr(0, max_server_name_length_v);
	const bool already_there = favorites.contains_manual(found->key, address);

	if (already_there && clipboard_at_start) {
		return false;
	}

	if (!already_there) {
		favorites.add_manual(found->key, address, name);
		added_favorite_note = name.empty() ? address : name;
		LOG("Added %x (%x) to the favorites from the clipboard.", added_favorite_note.value(), address);
	}

	/*
		Selected once the favorites list has it - as an edited favorite is.
	*/

	auto added = server_list_entry();
	added.favorite_only = true;
	added.unlisted = true;
	added.meta.server_public_key = found->key;
	added.favorite_address = address;

	current_tab = browser_tab::FAVORITES;
	selected_server = server_list_entry();
	reselect_after_refresh = added;
	scroll_once_to_selected = true;

	return true;
}

/*
	"Added to favorites", a check mark and the server's name - [OK].
*/

void browse_servers_gui_state::draw_added_favorite_modal() {
	if (!added_favorite_note.has_value()) {
		return;
	}

	const auto title = "Added to favorites###added_favorite";

	if (!ImGui::IsPopupOpen(title)) {
		ImGui::OpenPopup(title);
	}

	augs::imgui::center_next_window(ImGuiCond_Appearing);

	/*
		At least as wide as its title - a short name would cut it.
	*/

	{
		const auto& style = ImGui::GetStyle();
		const auto title_width = ImGui::CalcTextSize("Added to favorites").x + style.WindowPadding.x * 2 + style.FramePadding.x * 4;
		ImGui::SetNextWindowSizeConstraints(ImVec2(title_width, 0), ImVec2(FLT_MAX, FLT_MAX));
	}

	if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		const auto size = ImGui::GetTextLineHeight();
		const auto pos = ImGui::GetCursorScreenPos();
		auto* const draw_list = ImGui::GetWindowDrawList();
		const auto green = IM_COL32(34, 197, 76, 255);
		const auto thickness = std::max(1.5f, size * 0.15f);

		draw_list->AddLine(ImVec2(pos.x + size * 0.15f, pos.y + size * 0.55f), ImVec2(pos.x + size * 0.4f, pos.y + size * 0.8f), green, thickness);
		draw_list->AddLine(ImVec2(pos.x + size * 0.4f, pos.y + size * 0.8f), ImVec2(pos.x + size * 0.9f, pos.y + size * 0.2f), green, thickness);
		ImGui::Dummy(ImVec2(size, size));
		ImGui::SameLine();
		ImGui::TextUnformatted(added_favorite_note->c_str());

		ImGui::Separator();

		if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			added_favorite_note.reset();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}
