#include "augs/log.h"

#include "augs/templates/string_templates.h"
#include "augs/templates/container_templates.h"

#include "augs/gui/formatted_string.h"

namespace augs {
	namespace gui {
		namespace text {
			formatted_string format_recent_program_log(
				const baked_font& f,
				std::size_t lines_remaining
			) {
				formatted_string result;

				auto& entries = program_log::get_current().all_entries;

				lines_remaining = std::min(lines_remaining, entries.size());

				for (auto it = entries.end() - lines_remaining; it != entries.end(); ++it) {
					auto wstr = to_wstring((*it).text + "\n");
					concatenate(result, formatted_string{ wstr, { f, white /* rgba((*it).color) */ } });

					--lines_remaining;
				}

				return result;
			}

			formatted_string::operator std::wstring() const {
				const auto l = size();
				
				std::wstring out;
				out.reserve(l);

				for (const auto& c : *this) {
					out += c.unicode;
				}

				return out;
			}

			formatted_string& formatted_string::multiply_alpha(const float m) {
				for (auto& c : *this) {
					c.format.color.a = static_cast<unsigned char>(c.format.color.a * m);
				}

				return *this;
			}

			formatted_string& formatted_string::set_alpha(const rgba_channel m) {
				for (auto& c : *this) {
					c.format.color.a = m;
				}

				return *this;
			}

			formatted_string& formatted_string::set_alpha(const float m) {
				return set_alpha(to_0_255(m));
			}
			
			formatted_string& formatted_string::operator+=(const formatted_string& other) {
				insert(end(), other.begin(), other.end());
				return *this;
			}

			formatted_string::formatted_string(
				const std::wstring& str, 
				const style s
			) {
				reserve(str.size());

				for (const auto c : str) {
					emplace_back(s, c);
				}
			}

			formatted_char::formatted_char(
				const style format,
				const wchar_t unicode
			) : 
				format(format), 
				unicode(unicode) 
			{}

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

			void formatted_char::set_font(const baked_font& f) {
				format.font = std::addressof(f);
			}

			void formatted_char::set_color(const rgba col) {
				format.color = col;
			}

			bool formatted_char::operator==(const formatted_char& second) const {
				return unicode == second.unicode && format == second.format;
			}

			style::style(
				const baked_font& f, 
				const rgba c
			) : 
				font(std::addressof(f)), 
				color(c) 
			{}

			style::style(const formatted_char& c) : style(c.format)
			{}

			style::operator formatted_char() {
				return formatted_char(*this);
			}

			bool style::operator==(const style& b) const {
				return color == b.color && font == b.font;
			}
		}
	}
}
