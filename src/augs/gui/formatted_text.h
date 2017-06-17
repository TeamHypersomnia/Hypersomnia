#pragma once
#include <string>

#include "augs/graphics/rgba.h"
#include "game/assets/font_id.h"

namespace augs {
	namespace gui {
		namespace text {
			struct formatted_char;

			struct style {
				assets::font_id font;
				rgba color;
				style(assets::font_id = assets::font_id::GUI_FONT, rgba = rgba());
				style(const formatted_char&);
				operator formatted_char();

				bool operator==(const style& b) const;
			};

			struct formatted_char {
				style format;
				wchar_t unicode = 0;

				void set(
					const wchar_t code,
					const style format
				);

				void set_format(const style format);
				void set_font(const assets::font_id f);
				void set_color(const rgba col);

				bool operator==(const formatted_char& b) const;
			};

			using formatted_string = std::basic_string<formatted_char>;

			formatted_string format_recent_global_log(
				const assets::font_id,
				unsigned max_lines = 40
			);

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