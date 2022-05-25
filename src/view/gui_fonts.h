#pragma once
#include <compare>

namespace augs {
	struct baked_font;
	struct font_loading_input;
}

template <class What>
struct per_gui_font {
	// GEN INTROSPECTOR struct per_gui_font class What
	What gui;
	What larger_gui;
	What medium_numbers;
	What large_numbers;
	What very_large_numbers;
	// END GEN INTROSPECTOR

	bool operator==(const per_gui_font<What>&) const = default;
};

using all_gui_fonts_inputs = per_gui_font<augs::font_loading_input>;
using all_loaded_gui_fonts = per_gui_font<augs::baked_font>;
