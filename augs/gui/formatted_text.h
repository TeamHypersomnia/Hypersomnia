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

			typedef std::basic_string<formatted_char> formatted_string;
			
			formatted_string multiply_alpha(formatted_string, const float);
			formatted_string set_alpha(formatted_string, const float);

			std::wstring formatted_string_to_wstring(const formatted_string& f);
			
			formatted_string format_as_bbcode(const std::string&, const style default_style);
			formatted_string format_as_bbcode(const std::wstring&, const style default_style);
			
			formatted_string format(const std::wstring&, const style);
			void format(const std::wstring&, const style, formatted_string&);
		}
	}
}