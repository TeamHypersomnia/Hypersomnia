#pragma once

namespace augs {
	struct baked_font;
	struct font_loading_input;
}

template <class What>
struct per_gui_font {
	// GEN INTROSPECTOR struct per_gui_font class What
	What gui;
	What larger_gui;
	What large_numbers;
	// END GEN INTROSPECTOR
};

using all_gui_fonts_inputs = per_gui_font<augs::font_loading_input>;
using all_loaded_gui_fonts = per_gui_font<augs::baked_font>;
