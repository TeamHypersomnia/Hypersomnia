#include "application/gui/client/chat_gui.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "application/setups/client/client_vars.h"
#include "view/faction_view_settings.h"
#include "augs/gui/text/printer.h"
#include "augs/drawing/drawing.h"
#include "augs/string/format_enum.h"
#include "game/messages/game_notification.h"
#include "augs/templates/identity_templates.h"

const auto standard_gray_v = rgba(200, 200, 200, 255);

bool chat_gui_state::add_entry_from_game_notification(
	const net_time_t timestamp,
	const messages::game_notification& msg,
	const mode_player_id current_mode_id
) {
	using namespace messages;

	auto do_entry = [&](const auto& str, const rgba& col) {
		chat_gui_entry new_entry;
		new_entry.timestamp = timestamp;
		new_entry.author = "";
		new_entry.message = str;
		new_entry.overridden_message_color = col;

		add_entry(std::move(new_entry));
	};

	auto handle_payload = [&](const auto& payload) {
		using P = remove_cref<decltype(payload)>;
		using F = faction_choice;

		if constexpr(std::is_same_v<P, F>) {
			auto make_entry = [&](auto&&... args) {
				if (current_mode_id != msg.subject_mode_id) {
					return;
				}

				do_entry(
					typesafe_sprintf(std::forward<decltype(args)>(args)...),
					orange
				);
			};

			auto make_public_entry = [&](auto&&... args) {
				do_entry(
					typesafe_sprintf(std::forward<decltype(args)>(args)...),
					standard_gray_v
				);
			};

			const auto result = payload.result;
			const auto target = payload.target_faction;

			using FR = faction_choice_result;

			switch (result) {
				case FR::FAILED:
					make_entry("Unknown problem encountered while trying to change team.");
					break;
				case FR::THE_SAME:
#if 0
					if (target == faction_type::SPECTATOR) {
						make_entry("You are already a %x", format_enum(target));
					}
					else {
						make_entry("You are already in %x", format_enum(target));
					}
#endif
					break;
				case FR::CHOOSING_TOO_FAST:
					make_entry("You are choosing too fast! Wait for the next round."); 
					break;
				case FR::BEST_BALANCE_ALREADY:
					make_entry("The teams are already balanced."); 
					break;
				case FR::TEAM_IS_FULL:
					make_entry("You can't join %x - it is full.", target); 
					break;
				case FR::GAME_IS_FULL:
					make_entry("You can't join %x - the game is full.", target); 
					break;
				case FR::NEED_TO_BE_ALIVE_FOR_SPECTATOR:
					make_entry("You can join Spectators only when you have a character to sacrifice.", target); 
					break;
				case FR::CHANGED:
					if (target == faction_type::SPECTATOR) {
						make_public_entry("%x has joined the Spectators.", msg.subject_name, target); 
					}
					else {
						make_public_entry("%x has joined the %x.", msg.subject_name, format_enum(target)); 
					}

					break;
			}

			return true;
		}
		else if constexpr(std::is_same_v<P, joined_or_left>) {
			const auto action = [&]() {
				if (payload == joined_or_left::JOINED) {
					return "connected";
				}

				return "disconnected";
			}();

			do_entry(
				typesafe_sprintf("%x %x.", msg.subject_name, action),
				standard_gray_v
			);

			return true;
		}
		else {
			static_assert(always_false_v<P>);
			return false;
		}
	};

	return std::visit(handle_payload, msg.payload);
}

std::string chat_gui_entry::get_author_string() const {
	auto result = author;

	if (faction_specific) {
		result += std::string(" (") + std::string(augs::enum_to_string(author_faction)) + ")";
	}

	if (result.size() > 0) {
		result += ": ";
	}

	return result;
}

chat_gui_entry::operator std::string() const {
	return get_author_string() + message;
}

chat_gui_entry chat_gui_entry::from(
	const ::server_broadcasted_chat& payload,
	const net_time_t timestamp,
	const std::string& author,
	const faction_type author_faction
) {
	chat_gui_entry new_entry;

	new_entry.timestamp = timestamp;
	new_entry.author_faction = author_faction;

	const auto message_str = std::string(payload.message);

	new_entry.author = author;
	new_entry.message = message_str;

	if (author.empty()) {
		new_entry.overridden_message_color = rgba(200, 200, 200, 255);
	}

	switch (payload.target) {
		case chat_target_type::KICK:
			new_entry.author.clear();
			new_entry.message = typesafe_sprintf("%x was kicked from the server.\nReason: %x", author, message_str);
			new_entry.overridden_message_color = rgba(255, 100, 30, 255);
			break;

		case chat_target_type::BAN:
			new_entry.author.clear();
			new_entry.message = typesafe_sprintf("%x was banned from the server.\nReason: %x", author, message_str);
			new_entry.overridden_message_color = rgba(255, 100, 30, 255);
			break;

		case chat_target_type::INFO:
			new_entry.overridden_message_color = yellow;

		case chat_target_type::INFO_CRITICAL:
			new_entry.overridden_message_color = orange;

		default:
			break;
	}

	new_entry.faction_specific = payload.target == chat_target_type::TEAM_ONLY;
	return new_entry;
}

void chat_gui_state::open_input_bar(const chat_target_type t) {
	show = true;
	target = t;

	ImGui::SetWindowFocus("ChatWindow");
}

bool chat_gui_state::perform_input_bar(const client_chat_settings& vars) {
	using namespace augs::imgui;

	if (!show) {
		return false;
	}

	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);

	const auto size = vec2 {
		static_cast<float>(vars.chat_window_width),
		ImGui::GetTextLineHeight() * 3.f
	};

	const auto window_offset = vars.chat_window_offset;
	const auto window_pos = vec2(window_offset.x, screen_size.y - size.y - window_offset.y);

	ImGui::SetNextWindowPos(ImVec2(window_pos));

	auto go_to_entity = scoped_window(
		"ChatWindow", 
		&show,
		ImGuiWindowFlags_NoTitleBar 
		| ImGuiWindowFlags_NoResize 
		| ImGuiWindowFlags_NoMove 
		| ImGuiWindowFlags_NoScrollbar 
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_AlwaysAutoResize
	);

	bool was_acquired = false;
	
	if (ImGui::GetCurrentWindow()->GetID("##ChatInput") != GImGui->ActiveId) {
		ImGui::SetKeyboardFocusHere();
		was_acquired = true;
	}

	auto scope = augs::scope_guard([&](){
		if (!was_acquired && ImGui::GetCurrentWindow()->GetID("##ChatInput") != GImGui->ActiveId) {
			show = false;
		}
	});

	std::string label;

	switch (target) {
		case chat_target_type::GENERAL: label = "(Say to all)"; break;
		case chat_target_type::TEAM_ONLY: label = "(Say to team)"; break;
		default: break;
	}

	text_disabled(label);

	ImGui::SameLine();

	{
		auto scope = augs::imgui::scoped_item_width(size.x);

		if (input_text<max_chat_message_length_v>("##ChatInput", current_message, ImGuiInputTextFlags_EnterReturnsTrue)) {
			show = false;

			if (current_message.size() > 0) {
				return true;
			}
		}
	}

	return false;
}

void chat_gui_state::draw_recent_messages(
	const augs::drawer drawer,
	const client_chat_settings& vars,
   	const faction_view_settings& faction_view,
	const augs::baked_font& gui_font,
   	const net_time_t current_time
) const {
	using namespace augs::gui::text;

	const auto entries_to_show = std::min(
		history.size(), 
		static_cast<std::size_t>(vars.show_recent_chat_messages_num)
	);

	const auto now = current_time;

	const auto starting_i = [&]() {
		auto i = history.size() - entries_to_show;

		while (i < history.size() && now - history[i].timestamp >= vars.keep_recent_chat_messages_for_seconds) {
			++i;
		}

		return static_cast<int>(i);
	}();

	auto get_col = [&](const auto& faction) {
		return faction_view.colors[faction].standard;
	};

	auto colored = [&](const auto& text, const auto& c) {
		const auto text_style = style(gui_font, c);
		return formatted_string(text, text_style);
	};

	const auto wrapping = vars.chat_window_width;

	auto calc_size = [&](const auto& text) { 
		return get_text_bbox(colored(text, white), wrapping);
	};

	const auto size = vec2 {
		static_cast<float>(vars.chat_window_width),
		ImGui::GetTextLineHeight() * 3.f
	};

	const auto window_offset = vars.chat_window_offset;
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);
	const auto window_pos = vec2(window_offset.x, screen_size.y - size.y - window_offset.y);

	auto pen = window_pos;

	for (int i = history.size() - 1; i >= starting_i; --i) {
		const auto& entry = history[i];

		const auto author_text = entry.get_author_string();

		const auto author_col = get_col(entry.author_faction);
		const auto message_col = entry.overridden_message_color == rgba::zero ? white : entry.overridden_message_color;

		const auto total_text = colored(author_text, author_col) + colored(entry.message, message_col);

		const auto bbox = calc_size(total_text);
		pen.y -= bbox.y + 1;

		print_stroked(
			drawer,
			pen,
			total_text,
			augs::ralign_flags{},
			black,
			wrapping
		);
	}
}
