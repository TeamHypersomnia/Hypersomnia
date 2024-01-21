#include "application/gui/client/client_gui_state.h"
#include "augs/templates/container_templates.h"

bool client_gui_state::requires_cursor() const {
	return rcon.show;
}

bool client_gui_state::control(const handle_input_before_game_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (const auto it = mapped_or_nullptr(in.app_controls, key)) {
			if (*it == general_gui_intent_type::SERVER_ADMIN_PANEL) {
				auto& s = rcon.show;
				s = !s;
				return true;
			}
			
			if (*it == general_gui_intent_type::EXECUTE_RCON_GAME_COMMANDS) {
				rcon.request_execute_custom_game_commands = true;
				return true;
			}

			auto invoke_chat = [&](const chat_target_type t) {
				chat.open_input_bar(t);
			};

			if (*it == general_gui_intent_type::CHAT) {
				invoke_chat(chat_target_type::GENERAL);
				return true;
			}

			if (*it == general_gui_intent_type::CHAT_COMMAND) {
				invoke_chat(chat_target_type::GENERAL);
				chat.set_command = true;
				return true;
			}

			if (*it == general_gui_intent_type::TEAM_CHAT) {
				invoke_chat(chat_target_type::TEAM_ONLY);
				return true;
			}
		}
	}

	return false;
}
