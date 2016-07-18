#pragma once
#include "augs/graphics/pixel.h"
#include "game/assets/font_id.h"

namespace augs {
	namespace gui {
		namespace text {
			struct formatted_char {
				assets::font_id font_used;
				wchar_t c;
				unsigned char r, g, b, a;
				void set(wchar_t, assets::font_id = assets::font_id::GUI_FONT, const rgba& = rgba());
				void set(assets::font_id = assets::font_id::GUI_FONT, const rgba& = rgba());

				bool operator==(const formatted_char& b) const;
			};

			struct style {
				assets::font_id f;
				rgba color;
				style(assets::font_id = assets::font_id::GUI_FONT, rgba = rgba());
				style(const formatted_char&);
				operator formatted_char();
			};

			typedef std::basic_string<formatted_char> fstr;

			std::wstring formatted_string_to_wstring(const fstr& f);
			fstr simple_bbcode(std::wstring, style);
			fstr format(const std::wstring&, style);
			void format(const std::wstring&, style, fstr&);
			void format(const wchar_t*, style, fstr&);
			fstr format(const wchar_t*, style);
		}
	}
}