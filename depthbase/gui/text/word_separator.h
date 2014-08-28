#pragma once
#include "../system.h"

namespace db {
	namespace graphics {
		namespace gui {
			namespace text {
				struct word_separator {
					static int default_word_type(wchar_t);
					static bool default_is_newline(wchar_t);

					int (*word_type)(wchar_t);
					bool (*is_newline)(wchar_t);

					word_separator(int (*is_word)(wchar_t) = 0); /* 0 - default comparator */

					void set_default();

					/* max_left/right are introduced for word wrappers where no newlines are present inside strings;
					-1 means no boundaries

					versions without "alpha" argument will determine the alphanumeric flag
					and pass it as an argument to the latter versions, thus always making the offset at least one, 
					unless caret is at 0 or at str.length()
					*/
					unsigned get_left_word(const fstr&, int at) const;
					unsigned get_right_word(const fstr&, int at) const;

					unsigned get_left_word(const fstr&, int at, int max_left) const;
					unsigned get_right_word(const fstr&, int at, int max_right) const;

					unsigned get_left_word(const fstr&, int at, int max_left, int wordtype) const;
					unsigned get_right_word(const fstr&, int at, int max_right, int wordtype) const;
				};
			}
		}
	}
}