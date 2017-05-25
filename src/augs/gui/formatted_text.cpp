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
				using g_log = global_log;

				formatted_string result;

				lines_remaining = std::min(lines_remaining, g_log::all_entries.size());

				for (auto it = g_log::all_entries.end() - lines_remaining; it != g_log::all_entries.end(); ++it) {
					auto wstr = to_wstring((*it).text + "\n");
					result += format(wstr, style(f, rgba((*it).color)));

					--lines_remaining;
				}

				return result;
			}

			std::wstring formatted_string_to_wstring(
				const formatted_string& f
			) {
				const size_t l = f.size();
				
				std::wstring out;
				out.reserve(l);

				for (const auto& c : f) {
					out += c.c;
				}

				return out;
			}

			formatted_string multiply_alpha(
				formatted_string f, 
				const float m
			) {
				for (auto& c : f) {
					c.a = static_cast<unsigned char>(c.a * m);
				}

				return f;
			}

			formatted_string set_alpha(
				formatted_string f, 
				const float m
			) {
				for (auto& c : f) {
					c.a = static_cast<unsigned char>(m);
				}

				return f;
			}

			void format(
				const std::wstring& str, 
				const style s, 
				formatted_string& out
			) {
				out.clear();
				
				formatted_char ch;
				
				for (const auto& c : str) {
					ch.set(c, s.f, s.color);
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

			formatted_string format_as_bbcode(
				const std::string& str, 
				const style default_style
			) {
				return format_as_bbcode(to_wstring(str), default_style);
			}

			formatted_string format_as_bbcode(
				const std::wstring& input_str, 
				const style default_style
			) {
				typedef unsigned char flag;
				std::vector<flag> characters_to_skip;

				characters_to_skip.resize(input_str.size());
				std::fill(characters_to_skip.begin(), characters_to_skip.end(), 0u);

				auto output = format(input_str, default_style);

				const std::wstring opening_tag_body = L"[color=";
				const std::wstring closing_tag_body = L"[/color]";

				auto begin_of_opening_tag = input_str.find(opening_tag_body);

				while (begin_of_opening_tag != std::wstring::npos) {
					const auto begin_of_closing_tag = input_str.find(closing_tag_body, begin_of_opening_tag);
					const auto end_of_closing_tag = begin_of_closing_tag + closing_tag_body.length();

					const auto begin_of_argument = begin_of_opening_tag + opening_tag_body.size();
					const auto end_of_opening_tag = input_str.find(L']', begin_of_argument) + 1;
					const auto end_of_argument = end_of_opening_tag - 1;

					const auto argument = input_str.substr(begin_of_argument, end_of_argument - begin_of_argument);

					style new_style = default_style;

					if (argument == L"vsyellow") {
						new_style.color = vsyellow;
					}
					if (argument == L"vsblue") {
						new_style.color = vsblue;
					}
					if (argument == L"vscyan") {
						new_style.color = vscyan;
					}
					if (argument == L"cyan") {
						new_style.color = cyan;
					}
					if (argument == L"vsgreen") {
						new_style.color = vsgreen;
					}
					if (argument == L"green") {
						new_style.color = green;
					}
					if (argument == L"vsdarkgray") {
						new_style.color = vsdarkgray;
					}
					if (argument == L"vslightgray") {
						new_style.color = vslightgray;
					}
					if (argument == L"violet") {
						new_style.color = violet;
					}
					if (argument == L"red") {
						new_style.color = red;
					}
					if (argument == L"yellow") {
						new_style.color = yellow;
					}
					if (argument == L"white") {
						new_style.color = white;
					}
					if (argument == L"orange") {
						new_style.color = orange;
					}
					if (argument == L"turquoise") {
						new_style.color = turquoise;
					}

					for (size_t i = begin_of_opening_tag; i < end_of_opening_tag; ++i) {
						characters_to_skip[i] = 1u;
					}

					for (size_t i = begin_of_closing_tag; i < end_of_closing_tag; ++i) {
						characters_to_skip[i] = 1u;
					}

					for (size_t c = end_of_opening_tag; c < begin_of_closing_tag; ++c) {
						output[c].set(new_style.f, new_style.color);
					}

					begin_of_opening_tag = input_str.find(opening_tag_body, end_of_closing_tag);
				}

				size_t idx = 0;

				erase_if(output, [&characters_to_skip, &idx](auto){ return characters_to_skip[idx++] != 0u; });

				return output;
			}

			void formatted_char::set(
				const wchar_t ch, 
				const assets::font_id f, 
				const rgba& p
			) {
				font_used = f;
				c = ch;
				r = p.r;
				g = p.g;
				b = p.b;
				a = p.a;
			}

			void formatted_char::set(
				const assets::font_id f, 
				const rgba& p
			) {
				font_used = f;
				r = p.r;
				g = p.g;
				b = p.b;
				a = p.a;
			}

			bool formatted_char::operator==(const formatted_char& second) const {
				return font_used == second.font_used && c == second.c;
			}

			style::style(
				const assets::font_id f, 
				const rgba c
			) : 
				f(f), 
				color(c) 
			{
			}

			style::style(const formatted_char& c) : 
				f(c.font_used), 
				color(
					rgba(
						c.r, 
						c.g, 
						c.b, 
						c.a
					)
				) 
			{
			}

			style::operator formatted_char() {
				formatted_char c;
				c.set(f, color);
				return c;
			}
		}
	}
}
