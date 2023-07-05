#pragma once

template <class T>
void hud_message_2_players(
	const const_logic_step step,
	std::string preffix,
	std::string mid,
	std::string suffix,
	const T* first,
	const T* second,
	bool bbcode = false
) {
	messages::two_player_message msg;

	if (first) {
		msg.first_name = first->get_nickname();
		msg.first_faction = first->get_faction();
	}

	if (second) {
		msg.second_name = second->get_nickname();
		msg.second_faction = second->get_faction();
	}

	msg.preffix = std::move(preffix);
	msg.mid = std::move(mid);
	msg.suffix = std::move(suffix);
	msg.bbcode = bbcode;

	step.post_message(messages::hud_message { std::move(msg) });
}

template <class T>
void hud_message_1_player(
	const const_logic_step step,
	std::string preffix,
	std::string suffix,
	const T* first,
	bool bbcode = false
) {
	hud_message_2_players(step, preffix, "", suffix, first, (T*)nullptr, bbcode);
}

