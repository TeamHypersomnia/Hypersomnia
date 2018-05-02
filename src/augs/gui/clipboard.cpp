#include "clipboard.h"
#include "augs/ensure.h"
#include "augs/window_framework/platform_utils.h"

namespace augs {
	namespace gui {
		bool clipboard::is_clipboard_own() const {
			ensure(false);
			return false;
		}

		void paste_clipboard_formatted(text::formatted_string&, text::formatted_char) {
			ensure(false);
		}

		void clipboard::copy_clipboard(const text::formatted_string&) {
			ensure(false);
		}

		void clipboard::change_clipboard() {
			ensure(false);
		}
	}
}
