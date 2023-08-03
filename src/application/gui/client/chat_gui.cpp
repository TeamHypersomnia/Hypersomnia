#include "application/gui/client/chat_gui.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "application/setups/client/client_vars.h"
#include "view/faction_view_settings.h"
#include "augs/graphics/renderer.h"
#include "augs/gui/text/printer.h"
#include "augs/drawing/drawing.h"
#include "augs/string/format_enum.h"
#include "game/messages/mode_notification.h"
#include "augs/templates/identity_templates.h"

void LOG_NOFORMAT(const std::string& f);

const auto standard_gray_v = rgba(200, 200, 200, 255);

bool chat_gui_state::add_entry_from_mode_notification(
	const net_time_t timestamp,
	const messages::mode_notification& msg,
	const mode_player_id current_mode_id
) {
	using namespace messages;

	auto do_entry = [&](const auto& str, const rgba& col) {
		chat_gui_entry new_entry;
		new_entry.timestamp = timestamp;
		new_entry.author = "";
		new_entry.message = str;
		new_entry.overridden_message_color = col;

		LOG_NOFORMAT(new_entry.operator std::string());
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

				case FR::FAILED:
				default:
					make_entry("Unknown problem encountered while trying to change team.");
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

		case chat_target_type::DOWNLOADING_FILES:
			new_entry.author.clear();
			new_entry.message = typesafe_sprintf("%x is downloading files.", author);
			new_entry.overridden_message_color = yellow;
			break;

		case chat_target_type::DOWNLOADING_FILES_DIRECTLY:
			new_entry.author.clear();
			new_entry.message = typesafe_sprintf("%x is downloading files directly over UDP.", author);
			new_entry.overridden_message_color = yellow;
			break;

		case chat_target_type::FINISHED_DOWNLOADING:
			new_entry.author.clear();
			new_entry.message = typesafe_sprintf("%x finished downloading files.", author);
			new_entry.overridden_message_color = green;
			break;

		case chat_target_type::SERVER_SHUTTING_DOWN:
			new_entry.author.clear();
			new_entry.message = "Server shutting down.";
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
		case chat_target_type::GENERAL: label = " (Say to all)"; break;
		case chat_target_type::TEAM_ONLY: label = " (Say to team)"; break;
		default: break;
	}

	const auto local_pos = ImGui::GetCursorPos();

	std::array<char, max_chat_message_length_v> buf;
	buf[0] = '\0';

	{
		auto scope = augs::imgui::scoped_item_width(size.x);

		if (input_text(buf, "##ChatInput", current_message, ImGuiInputTextFlags_EnterReturnsTrue)) {
			show = false;

			if (current_message.size() > 0) {
				return true;
			}
		}
	}

	ImGui::SetCursorPos(local_pos);

	if (buf[0] == '\0') {
		text_color(label, rgba(220, 255, 255, 220));
	}

	return false;
}

void chat_gui_state::draw_recent_messages(
	const augs::drawer_with_default drawer,
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

		if (!show) {
			while (i < history.size() && now - history[i].timestamp >= vars.keep_recent_chat_messages_for_seconds) {
				++i;
			}
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


	const auto window_padding = ImGui::GetStyle().WindowPadding;
	const auto wrapping = vars.chat_window_width;
	const auto drawn_window_width = vars.chat_window_width + 2 * window_padding.x;

	auto calc_size = [&](const auto& text) { 
		return get_text_bbox(colored(text, white), wrapping);
	};

	const auto size = vec2 {
		static_cast<float>(wrapping),
		ImGui::GetTextLineHeight() * 3.f
	};

	const auto window_offset = vars.chat_window_offset;
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);
	const auto window_pos = vec2(window_offset.x, screen_size.y - size.y - window_offset.y);

	auto pen = window_pos + vec2(window_padding.x, -window_padding.y);

	auto& buf = drawer.output_buffer;
	const auto current_i = buf.size();

	if (show) {
		// 8 not 10 to avoid drawing the bottom border
		buf.resize(buf.size() + 8);
	}

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

	if (show) {
		augs::vertex_triangle_buffer aabb_buf;
		auto aabb_drawer = augs::drawer_with_default { aabb_buf, drawer.default_texture };
		aabb_drawer.aabb_with_border(ltrb::from_points(pen - vec2(window_padding.x, window_padding.y), window_pos + vec2(drawn_window_width, 1)), vars.background_color, vars.border_color, border_input { -1, 0 } );

		for (std::size_t i = 0; i < 8 && i < aabb_buf.size(); ++i) {
			buf[current_i + i] = aabb_buf[i];
		}
	}
}
