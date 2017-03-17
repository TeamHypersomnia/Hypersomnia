#include "clipboard.h"
#include "augs/window_framework/platform_utils.h"

namespace augs {
	namespace gui {
		bool clipboard::is_clipboard_own() const {
			return own_clip;
		}

		void paste_clipboard_formatted(text::formatted_string& out, text::formatted_char f) {
			//auto w = window::get_data_from_clipboard();
			//size_t len = w.length();
			//out.clear();
			//out.reserve(len);
			//for (size_t i = 0; i < len; ++i) {
			//	f.c = w[i];
			//	out += f;
			//}
		}

		void clipboard::copy_clipboard(const text::formatted_string& s) {
			contents = s;
			own_copy = true;
			own_clip = true;
			// window::set_clipboard_data(formatted_string_to_wstring(s));
		}

		void clipboard::change_clipboard() {
			if (!own_copy && fetch_clipboard) {
				text::formatted_char ch;
				ch.set(0, assets::font_id::GUI_FONT, rgba(0, 0, 0, 255));
				paste_clipboard_formatted(contents, ch);
				own_clip = false;
			}

			own_copy = false;
		}
	}
}
