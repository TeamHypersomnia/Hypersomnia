#pragma once
#include "word_separator.h"

namespace db {
	namespace graphics {
		namespace gui {
			namespace text {
				bool word_separator::default_is_newline(wchar_t i) {
					return (i == 0x000A || i == 0x000D);
				}

				int word_separator::default_word_type(wchar_t c) {
					if(iswspace(c)) return 0;
					else if(iswalnum(c)) return 1;
					else if(iswpunct(c)) return 2;
					else return 3;
				}

				void word_separator::set_default() {
					word_type =    default_word_type;
					is_newline = default_is_newline;
				}

				word_separator::word_separator(int (*word_type)(wchar_t)) : word_type(word_type) {
					if(word_type == 0) set_default();
				}

				unsigned word_separator::get_left_word(const fstr& str, int at) const {
					return get_left_word(str, at, -1);
				}

				unsigned word_separator::get_right_word(const fstr& str, int at) const {
					return get_right_word(str, at, -1);
				}

				unsigned word_separator::get_left_word(const fstr& _str, int at, int max_left) const {
					if(max_left == -1) max_left = 0;
					if(_str.empty() || at <= int(max_left)) return 0;

					unsigned result = get_left_word(_str, at, max_left, word_type(_str[at ? at - 1 : 0].c));
					return result == 0 ? 
						1 /* newline encountered */
						: result;
				}

				unsigned word_separator::get_right_word(const fstr& _str, int at, int max_right) const {
					if(max_right == -1) max_right = _str.length();
					if(_str.empty() || at >= int(max_right)) return 0;

					unsigned result = get_right_word(_str, at, max_right, word_type(_str[at].c));
					return result == 0 ? 
						1 /* newline encountered */
						: result;
				}

				unsigned word_separator::get_left_word(const fstr& _str, int at, int max_left, int wordtype) const {
					if(max_left == -1) max_left = 0;
					if(_str.empty() || at <= int(max_left)) return 0;

					unsigned offset = 0;
					while(at > max_left) {
						if(!is_newline(_str[at ? at - 1 : 0].c) && word_type(_str[at ? at - 1 : 0].c) == wordtype) {
							++offset;
							--at;
						}
						else break;
					}

					return offset;
				}

				unsigned word_separator::get_right_word(const fstr& _str, int at, int max_right, int wordtype) const {
					if(max_right == -1) max_right = _str.length();
					if(_str.empty() || at >= int(max_right)) return 0;

					unsigned offset = 0;
					while(at < max_right) {
						if(!is_newline(_str[at].c) && word_type(_str[at].c) == wordtype) {
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
}