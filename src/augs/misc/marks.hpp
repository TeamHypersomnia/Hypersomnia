#pragma once
#include "augs/misc/marks.h"

namespace augs {
	template <class T>
	void marks<T>::close() {
		state = augs::marks_state::NONE;
		eat_keys = 0;
	}

	template <class T>
	typename marks<T>::result marks<T>::control(
		const event::change& ch,
		const T& current
	) {
		using namespace event;
		using namespace keys;
		using R = result;

		const auto only_fetch = R {
			marks_result_type::ONLY_FETCH_INPUT,
			previous
		};

		const auto none = R {
			marks_result_type::NONE,
			previous
		};

		if (eat_keys && ch.was_any_key_pressed()) {
			--eat_keys;
			return only_fetch;
		}

		if (state == marks_state::NONE) {
			if (ch.was_pressed(key::M)) {
				state = marks_state::MARKING;
				return only_fetch;
			}

			if (ch.was_pressed(key::APOSTROPHE)) {
				state = marks_state::JUMPING;
				return only_fetch;
			}
		}

		if (state == marks_state::REMOVING) {
			if (ch.was_pressed(key::BACKSPACE)) {
				state = marks_state::MARKING;
				return only_fetch;
			}

			if (ch.msg == message::character) {
				const auto code_point = ch.data.character.code_point;

				if (code_point != '\'') {
					marks.erase(code_point);
				}
			}

			if (ch.get_key_change() != key_change::NO_CHANGE) {
				return only_fetch;
			}
		}

		if (state == marks_state::MARKING) {
			if (ch.was_pressed(key::DEL)) {
				marks.clear();
				return only_fetch;
			}

			if (ch.was_pressed(key::BACKSPACE)) {
				state = marks_state::REMOVING;
				return only_fetch;
			}

			if (ch.msg == message::character) {
				state = marks_state::NONE;

				const auto code_point = ch.data.character.code_point;

				if (code_point == '\'') {
					previous = current;
				}
				else {
					marks[code_point] = current;
				}

				eat_keys = 1;

				return only_fetch;
			}

			if (ch.get_key_change() != key_change::NO_CHANGE) {
				return only_fetch;
			}
		}

		if (state == marks_state::JUMPING) {
			auto jump_to = [&](const T& to) {
				state = marks_state::NONE;

				const auto result = R {
					marks_result_type::JUMPED,
					to
				};

				previous = current;

				return result;
			};

			if (ch.was_pressed(key::APOSTROPHE)) {
				return jump_to(previous);
			}

			if (ch.msg == message::character) {
				const auto code_point = ch.data.character.code_point;

				if (const auto entry = mapped_or_nullptr(marks, code_point)) {
					eat_keys = 1;
					return jump_to(*entry);
				}

				return only_fetch;
			}

			if (ch.get_key_change() != key_change::NO_CHANGE) {
				return only_fetch;
			}
		}

		return none;
	}
}
