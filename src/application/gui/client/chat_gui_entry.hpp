#pragma once

std::string chat_gui_entry::get_author_string(const bool streamer_mode) const {
	auto result = author;

	if (streamer_mode && result.size() > 0) {
		result = "Player";
	}

	if (faction_specific) {
		result += std::string(" (") + std::string(augs::enum_to_string(author_faction)) + ")";
	}

	if (result.size() > 0) {
		if (is_user_message) {
			result += ": ";
		}
		else {
			result += " ";
		}
	}

	return result;
}

std::string chat_gui_entry::to_string() const {
	return get_author_string(false) + message;
}

chat_gui_entry chat_gui_entry::from(
	const ::server_broadcasted_chat& payload,
	const net_time_t timestamp,
	const std::string& author,
	const faction_type author_faction,
	const bool is_my_message
) {
	chat_gui_entry new_entry;

	new_entry.timestamp = timestamp;
	new_entry.author_faction = author_faction;

	const auto message_str = std::string(payload.message);

	new_entry.author = author;
	new_entry.message = message_str;
	new_entry.is_my_message = is_my_message;

	if (author.empty()) {
		new_entry.overridden_message_color = rgba(200, 200, 200, 255);
	}

	switch (payload.target) {
		case chat_target_type::KICK:
			new_entry.message = typesafe_sprintf("was kicked from the server.\nReason: %x", message_str);
			new_entry.overridden_message_color = rgba(255, 100, 30, 255);
			break;

		case chat_target_type::DOWNLOADING_FILES:
			new_entry.message = "is downloading files.";
			new_entry.overridden_message_color = yellow;
			break;

		case chat_target_type::DOWNLOADING_FILES_DIRECTLY:
			new_entry.message = "is downloading files directly over UDP.";
			new_entry.overridden_message_color = yellow;
			break;

		case chat_target_type::FINISHED_DOWNLOADING:
			new_entry.message = "finished downloading files.";
			new_entry.overridden_message_color = green;
			break;

		case chat_target_type::SERVER_SHUTTING_DOWN:
			new_entry.author.clear();
			new_entry.message = "Server shutting down.";
			break;

		case chat_target_type::BAN:
			new_entry.author = author;
			new_entry.message = typesafe_sprintf("was banned from the server.\nReason: %x", message_str);
			new_entry.overridden_message_color = rgba(255, 100, 30, 255);
			break;

		case chat_target_type::INFO:
			new_entry.author.clear();
			new_entry.overridden_message_color = yellow;
			break;

		case chat_target_type::INFO_CRITICAL:
			new_entry.author.clear();
			new_entry.overridden_message_color = orange;
			break;

		case chat_target_type::GENERAL:
		case chat_target_type::TEAM_ONLY:
			new_entry.is_user_message = true;
			break;

		default:
			break;
	}

	new_entry.faction_specific = payload.target == chat_target_type::TEAM_ONLY;
	return new_entry;
}
