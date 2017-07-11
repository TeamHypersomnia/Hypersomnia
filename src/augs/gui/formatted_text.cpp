#include "formatted_text.h"
#include <algorithm>
#include <cstring>
#include <vector>

#include "augs/templates/string_templates.h"
#include "augs/templates/container_templates.h"
#include "augs/log.h"

namespace augs {
	namespace gui {
		namespace text {
			formatted_string format_recent_global_log(
				const assets::font_id f,
				unsigned lines_remaining
			) {
				formatted_string result;

				lines_remaining = std::min(lines_remaining, global_log::all_entries.size());

				for (auto it = global_log::all_entries.end() - lines_remaining; it != global_log::all_entries.end(); ++it) {
					auto wstr = to_wstring((*it).text + "\n");
					result += format(wstr, style(f, rgba((*it).color)));

					--lines_remaining;
				}

				return result;
			}

			std::wstring formatted_string_to_wstring(
				const formatted_string& f
			) {
				const std::size_t l = f.size();
				
				std::wstring out;
				out.reserve(l);

				for (const auto& c : f) {
					out += c.unicode;
				}

				return out;
			}

			formatted_string multiply_alpha(
				formatted_string f, 
				const float m
			) {
				for (auto& c : f) {
					c.format.color.a = static_cast<unsigned char>(c.format.color.a * m);
				}

				return f;
			}

			formatted_string set_alpha(
				formatted_string f, 
				const rgba_channel m
			) {
				for (auto& c : f) {
					c.format.color.a = m;
				}

				return f;
			}

			formatted_string set_alpha(
				formatted_string f,
				const float m
			) {
				return set_alpha(f, static_cast<rgba_channel>(255 * m));
			}

			void format(
				const std::wstring& str, 
				const style s, 
				formatted_string& out
			) {
				out.clear();
				
				formatted_char ch;
				
				for (const auto& c : str) {
					ch.set(c, s);
					out.append(1, ch);
				}
			}

			formatted_string format(
				const std::wstring& str, 
				const style s
			) {
				formatted_string out;
				format(str, s, out);
				return out;
			}

			void formatted_char::set(
				const wchar_t code,
				const style _format
			) {
				unicode = code;
				set_format(_format);
			}

			void formatted_char::set_format(const style _format) {
				format = _format;
			}

			void formatted_char::set_font(const assets::font_id f) {
				format.font = f;
			}

			void formatted_char::set_color(const rgba col) {
				format.color = col;
			}

			bool formatted_char::operator==(const formatted_char& second) const {
				return unicode == second.unicode && format == second.format;
			}

			style::style(
				const assets::font_id f, 
				const rgba c
			) : 
				font(f), 
				color(c) 
			{
			}

			style::style(const formatted_char& c) : style(c.format)
			{}

			style::operator formatted_char() {
				formatted_char c;
				c.set_format(*this);
				return c;
			}

			bool style::operator==(const style& b) const {
				return color == b.color && font == b.font;
			}
		}
	}
}
