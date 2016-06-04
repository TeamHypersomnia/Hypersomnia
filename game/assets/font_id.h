#pragma once

namespace assets {
	enum font_id {
		INVALID,
		GUI_FONT
	};
}

namespace augs {
	class font;
}

augs::font& operator*(const assets::font_id& id);
bool operator!(const assets::font_id& id);