#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_status_icons.h"
#include "augs/network/network_types.h"
#include "application/network/address_utils.h"
#include "application/gui/unlisted_server_fields.h"

std::optional<connect_target::server_identifiers> unlisted_server_fields::find(const std::string& text) {
	auto found = connect_target::find_server_identifiers(text);

	if (!found.has_value() || !::is_pasted_address(found->address)) {
		return std::nullopt;
	}

	/*
		As long as a server's own name at most - not cut inside a character.
	*/

	if (auto& n = found->name; n.size() > max_server_name_length_v) {
		n.resize(max_server_name_length_v);

		while (!n.empty() && (static_cast<unsigned char>(n.back()) & 0x80) != 0) {
			n.pop_back();
		}
	}

	return found;
}

bool unlisted_server_fields::take(const std::string& text) {
	if (const auto found = find(text)) {
		address = found->address;
		key = "0x" + augs::bytes_to_hex(found->key);

		if (!found->name.empty()) {
			name = found->name;
		}

		augs::imgui::stop_editing_text();
		return true;
	}

	return false;
}

bool unlisted_server_fields::autopaste(const char* const clipboard) {
	if (clipboard == nullptr || clipboard == last_clipboard || !take(clipboard)) {
		return false;
	}

	last_clipboard = clipboard;
	autopasted = std::array<std::string, 3> { address, key, name };
	return true;
}

bool unlisted_server_fields::take_paste() {
	const auto& io = ImGui::GetIO();

	if (!(io.KeyCtrl || io.KeySuper) || !ImGui::IsKeyPressed(ImGuiKey_V, false)) {
		return false;
	}

	const auto clipboard = ImGui::GetClipboardText();

	if (clipboard == nullptr) {
		return false;
	}

	const auto pasted = std::string(clipboard);

	if (take(pasted)) {
		return true;
	}

	if (key_active && connect_target::key_of(pasted).has_value()) {
		key = connect_target::trimmed(pasted);
		augs::imgui::stop_editing_text();
	}
	else if (address_active && ::is_pasted_address(pasted)) {
		address = connect_target::trimmed(pasted);
		augs::imgui::stop_editing_text();
		focus_key_next = !typed_key().has_value();
	}

	return false;
}

std::optional<augs::ed25519_public_key_type> unlisted_server_fields::typed_key() const {
	return connect_target::key_of(key);
}

std::string unlisted_server_fields::typed_address() const {
	return connect_target::without_default_port(address);
}

bool unlisted_server_fields::ready() const {
	return ::is_pasted_address(address) && typed_key().has_value();
}

bool unlisted_server_fields::is_autopasted() const {
	return autopasted.has_value() && *autopasted == std::array<std::string, 3> { address, key, name };
}

void unlisted_server_fields::status_line(
	const std::optional<std::pair<std::string, rgba>>& problem,
	const bool hints,
	const char* const hint
) const {
	using namespace augs::imgui;

	if (!key.empty() && !typed_key().has_value()) {
		text_color("The key should be 64 hex digits.", orange);
	}
	else if (problem.has_value()) {
		text_color(problem->first, problem->second);
	}
	else if (is_autopasted()) {
		checkmark_line("Auto-pasted from clipboard", green);
	}
	else if (!hints) {
		text(" ");
	}
	else if (ready()) {
		checkmark_line("OK", green);
	}
	else {
		text_disabled(hint);
	}
}
