#pragma once
#include <string>

#include "augs/graphics/rgba.h"
#include "augs/image/font.h"

namespace augs {
	namespace gui {
		namespace text {
			struct formatted_char;

			struct style {
				const baked_font* font;
				rgba color;
				
				style(const baked_font&, rgba = rgba());
				style(const formatted_char&);
				operator formatted_char();

				bool operator==(const style& b) const;
				
				const baked_font& get_font() const {
					return *font;
				}
			};

			struct formatted_char {
				style format;
				wchar_t unicode = 0;

				formatted_char(
					const style,
					const wchar_t = 0
				);

				void set(
					const wchar_t code,
					const style format
				);

				void set_format(const style format);
				void set_font(const baked_font& f);
				void set_color(const rgba col);

				bool operator==(const formatted_char& b) const;
			};

			struct formatted_string : public std::vector<formatted_char> {
				using self = formatted_string&;
				
				formatted_string() = default;
				formatted_string(const std::wstring&, const style);

				self multiply_alpha(const float);
				self set_alpha(const rgba_channel);
				self set_alpha(const float);

				formatted_string& operator+=(const formatted_string&);

				operator std::wstring() const;
			};

			formatted_string format_recent_global_log(
				const baked_font&,
				unsigned max_lines = 40
			);

			formatted_string from_bbcode(const std::string&, const style default_style);
			formatted_string from_bbcode(const std::wstring&, const style default_style);
		}
	}
}