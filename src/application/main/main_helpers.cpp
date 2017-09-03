#include "game/systems_audiovisual/game_gui_system.h"
#include "application/main/main_helpers.h"
#include "application/main/release_flags.h"

void switch_between_game_gui_and_back(
	game_intent_vector& intents,
	game_gui_system& gui,
	release_flags& flags
) {
	bool has_changed = false;

	erase_if(intents, [&](const game_intent& intent) {
		bool fetched = false;

		for (const auto& intent : intents) {
			if (intent.intent == intent_type::SWITCH_TO_GUI) {
				fetched = true;

				if (intent.is_pressed) {
					has_changed = true;
					gui.active = !gui.active;
				}
			}
		}

		return fetched;
	});

	if (has_changed) {
		flags.mouse = true;
	}
}

void switch_developer_console(
	game_intent_vector& intents,
	bool& flag
) {
	erase_if(intents, [&](const game_intent& intent) {
		bool fetched = false;

		if (intent.intent == intent_type::SWITCH_DEVELOPER_CONSOLE) {
			fetched = true;

			if (intent.is_pressed) {
				flag = !flag;
			}
		}

		return fetched;
	});
}

void switch_weapon_laser(
	game_intent_vector& intents,
	bool& flag
) {
	erase_if(intents, [&](const game_intent& intent) {
		bool fetched = false;

		if (intent.intent == intent_type::SWITCH_WEAPON_LASER) {
			fetched = true;

			if (intent.is_pressed) {
				flag = !flag;
			}
		}

		return fetched;
	});
}

void clear_debug_lines(
	game_intent_vector& intents,
	std::vector<augs::renderer::debug_line>& lines
) {
	erase_if(intents, [&](const game_intent& intent) {
		bool fetched = false;

		if (intent.intent == intent_type::CLEAR_DEBUG_LINES) {
			fetched = true;
			
			if (intent.is_pressed) {
				lines.clear();
			}
		}

		return fetched;
	});
}

void handle_exit_events(
	augs::local_entropy& entropy,
	bool& should_quit
) {
	erase_if(entropy, [&](const auto& n) {
		using namespace augs::event;
		bool fetched = false;

		if (
			n.msg == message::close
			|| n.msg == message::quit
			|| (n.msg == message::syskeydown && n.key.key == keys::key::F4)
		) {
			fetched = true;
			should_quit = true;
		}

		return fetched;
	});
}