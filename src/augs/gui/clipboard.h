#pragma once
#include "formatted_string.h"

namespace augs {
	namespace gui {
		class clipboard {
		public:
			bool fetch_clipboard = true;

			text::formatted_string contents;

			void change_clipboard();
			void copy_clipboard(const text::formatted_string&);

			bool is_clipboard_own() const;
		};

		void paste_clipboard_formatted(text::formatted_string& out, text::formatted_char);
	}
}
