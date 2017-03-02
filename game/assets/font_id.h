#pragma once

namespace assets {
	enum class font_id {
		INVALID,

		GUI_FONT,
		COUNT
	};
}

namespace augs {
	struct baked_font;
}

augs::baked_font& operator*(const assets::font_id& id);
bool operator!(const assets::font_id& id);