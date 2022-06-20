#include <mutex>
#include "augs/log.h"

#include "augs/string/string_templates.h"
#include "augs/templates/container_templates.h"

#include "augs/gui/formatted_string.h"

int ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);

extern std::mutex log_mutex;

namespace augs {
	namespace gui {
		namespace text {
			formatted_utf32_string::formatted_utf32_string(const formatted_string& utf8) {
				thread_local std::string s;
			   	s = utf8.operator std::string();
				reserve(s.size());

				static_assert(sizeof(unsigned int) == sizeof(utf32_point));

				auto in_text = s.data();
				auto in_text_end = s.data() + s.size();

				std::size_t color_idx = 0;

				while ((!in_text_end || in_text < in_text_end) && *in_text)
				{
					utf32_point c = 0xdeadbeef;

					const auto eaten = ImTextCharFromUtf8(&c, in_text, in_text_end);
					in_text += eaten;

					if (c == 0)
					break;

					push_back({ utf8[color_idx].format, c});
					color_idx += eaten;
				}
			}

			formatted_string format_recent_program_log(
				const baked_font& f,
				std::size_t lines_remaining
			) {
				formatted_string result;

				std::unique_lock<std::mutex> lock(log_mutex);

				const auto& current = program_log::get_current();
				auto& entries = current.all_entries;

				const auto init_logs_n = current.get_init_logs_count_nomutex();
				lines_remaining = std::min(lines_remaining, entries.size() - init_logs_n);

				for (auto it = entries.end() - lines_remaining; it != entries.end(); ++it) {
					const auto str = (*it).text + "\n";
					concatenate(result, formatted_string{ str, { f, white /* rgba((*it).color) */ } });

					--lines_remaining;
				}

				return result;
			}

			formatted_string::operator std::string() const {
				const auto l = size();
				
				thread_local std::string out;
				out.clear();

				out.reserve(l);

				for (const auto& c : *this) {
					out += c.utf_unit;
				}

				return out;
			}

			formatted_string& formatted_string::mult_alpha(const float m) {
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

			formatted_string formatted_string::operator+(const formatted_string& other) const {
				auto cpy = *this;
				cpy += other;
				return cpy;
			}

			formatted_string::formatted_string(
				const std::string& str, 
				const style s
			) {
				reserve(str.size());

				for (const auto c : str) {
					emplace_back(s, c);
				}
			}

			formatted_char::formatted_char(
				const style format,
				const char utf_unit
			) : 
				format(format), 
				utf_unit(utf_unit) 
			{}

			void formatted_char::set(
				const char code,
				const style _format
			) {
				utf_unit = code;
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
				return utf_unit == second.utf_unit && format == second.format;
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

			style::style(const formatted_utf32_char& c) : style(c.format)
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
