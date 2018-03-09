#include "word_separator.h"

namespace augs {
	namespace gui {
		namespace text {
			bool word_separator::default_is_newline(utf32_point i) {
				return (i == 0x000A || i == 0x000D);
			}

			int word_separator::default_word_type(utf32_point c) {
				if (iswspace(c)) return 0;
				else if (iswalnum(c)) return 1;
				else if (iswpunct(c)) return 2;
				else return 3;
			}

			void word_separator::set_default() {
				word_type = default_word_type;
				is_character_newline = default_is_newline;
			}

			word_separator::word_separator(int(*word_type)(utf32_point)) : word_type(word_type) {
				if (word_type == 0) set_default();
			}

			unsigned word_separator::get_left_word(const formatted_utf32_string& str, int at) const {
				return get_left_word(str, at, -1);
			}

			unsigned word_separator::get_right_word(const formatted_utf32_string& str, int at) const {
				return get_right_word(str, at, -1);
			}

			unsigned word_separator::get_left_word(const formatted_utf32_string& _str, int at, int max_left) const {
				if (max_left == -1) max_left = 0;
				if (_str.empty() || at <= int(max_left)) return 0;

				unsigned result = get_left_word(_str, at, max_left, word_type(_str[at ? at - 1 : 0].utf_unit));
				return result == 0 ?
					1 /* newline encountered */
					: result;
			}

			unsigned word_separator::get_right_word(const formatted_utf32_string& _str, int at, int max_right) const {
				if (max_right == -1) max_right = static_cast<int>(_str.size());
				if (_str.empty() || at >= int(max_right)) return 0;

				unsigned result = get_right_word(_str, at, max_right, word_type(_str[at].utf_unit));
				return result == 0 ?
					1 /* newline encountered */
					: result;
			}

			unsigned word_separator::get_left_word(const formatted_utf32_string& _str, int at, int max_left, int wordtype) const {
				if (max_left == -1) max_left = 0;
				if (_str.empty() || at <= int(max_left)) return 0;

				unsigned offset = 0;
				while (at > max_left) {
					if (!is_character_newline(_str[at ? at - 1 : 0].utf_unit) && word_type(_str[at ? at - 1 : 0].utf_unit) == wordtype) {
						++offset;
						--at;
					}
					else break;
				}

				return offset;
			}

			unsigned word_separator::get_right_word(const formatted_utf32_string& _str, int at, int max_right, int wordtype) const {
				if (max_right == -1) max_right = static_cast<int>(_str.size());
				if (_str.empty() || at >= int(max_right)) return 0;

				unsigned offset = 0;
				while (at < max_right) {
					if (!is_character_newline(_str[at].utf_unit) && word_type(_str[at].utf_unit) == wordtype) {
						++offset;
						++at;
					}
					else break;
				}

				return offset;
			}
		}
	}
}