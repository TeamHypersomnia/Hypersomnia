#pragma once

namespace assets {
	enum class font_id {
		INVALID,

		GUI_FONT,
		COUNT
	};
}

namespace augs {
	class font;
}

augs::font& operator*(const assets::font_id& id);
bool operator!(const assets::font_id& id);