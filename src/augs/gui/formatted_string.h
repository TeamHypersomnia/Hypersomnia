#pragma once
#include <string>
#include <vector>

#include "augs/graphics/rgba.h"
#include "augs/image/font.h"
#include "augs/utf32_point.h"

namespace augs {
	namespace gui {
		namespace text {
			struct formatted_char;
			struct formatted_utf32_char;

			struct style {
				const baked_font* font;
				rgba color;
				
				style(const baked_font&, rgba = white);
				style(const formatted_char&);
				style(const formatted_utf32_char&);
				operator formatted_char();

				bool operator==(const style& b) const;
				
				const baked_font& get_font() const {
					return *font;
				}
			};

			struct formatted_char {
				style format;
				char utf_unit = 0;

				formatted_char(
					const style,
					const char = 0
				);

				void set(
					const char code,
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
				formatted_string(const std::string&, const style);

				self mult_alpha(const float);
				self set_alpha(const rgba_channel);
				self set_alpha(const float);

				formatted_string operator+(const formatted_string&) const;
				formatted_string& operator+=(const formatted_string&);

				operator std::string() const;
			};

			struct formatted_utf32_char {
				style format;
				utf32_point utf_unit = 0;

				bool operator==(const formatted_utf32_char& b) const {
					return format == b.format && utf_unit == b.utf_unit;
				}
			};

			struct formatted_utf32_string : public std::vector<formatted_utf32_char> {
				formatted_utf32_string() = default;
				formatted_utf32_string(const formatted_string&);
			};

			formatted_string format_recent_program_log(
				const baked_font&,
				std::size_t max_lines = 40
			);

			formatted_string from_bbcode(const std::string&, const style default_style);
		}
	}
}