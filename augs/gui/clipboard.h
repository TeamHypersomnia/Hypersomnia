#pragma once
#include "formatted_text.h"

namespace augs {
	namespace gui {
		class clipboard {
			bool
				/* own_copy indicates whether the clipboard_change event comes from manual "copy_clipboard" or from external source */
				own_copy = false,
				own_clip = false;

		public:
			bool fetch_clipboard = true;

			text::fstr contents;

			void change_clipboard();
			void copy_clipboard(text::fstr&);

			bool is_clipboard_own() const;
		};
	}
}
