#include "formatted_text.h"
#include <algorithm>
#include <cstring>
#include <vector>

#include "augs/templates/string_templates.h"

namespace augs {
	namespace gui {
		namespace text {
			std::wstring formatted_string_to_wstring(const fstr& f) {
				size_t l = f.size();
				std::wstring ww;
				ww.reserve(l);
				for (size_t i = 0; i < l; ++i)
					ww += f[i].c;

				return ww;
			}

			fstr multiply_alpha(fstr f, const float m) {
				for (auto& c : f) {
					c.a = static_cast<unsigned char>(c.a * m);
				}

				return std::move(f);
			}

			fstr set_alpha(fstr f, const float m) {
				for (auto& c : f) {
					c.a = static_cast<unsigned char>(m);
				}

				return std::move(f);
			}

			void format(const wchar_t* _str, style s, fstr& out) {
				out.clear();
				formatted_char ch;
				int len = wcslen(_str);
				for (int i = 0; i < len; ++i) {
					ch.set(_str[i], s.f, s.color);
					out.append(1, ch);
				}
			}

			fstr format(const wchar_t* _str, style s) {
				fstr out;

				formatted_char ch;
				ch.font_used = s.f;
				int len = wcslen(_str);

				//out.reserve(len);
				for (int i = 0; i < len; ++i) {
					ch.set(_str[i], s.f, s.color);
					out.append(1, ch);
				}

				return out;
			}

			void format(const std::wstring& _str, style s, fstr& out) {
				out.clear();
				formatted_char ch;
				int len = _str.length();
				for (int i = 0; i < len; ++i) {
					ch.set(_str[i], s.f, s.color);
					out.append(1, ch);
				}
			}

			fstr format(const std::wstring& _str, style s) {
				fstr out;

				formatted_char ch;
				ch.font_used = s.f;
				int len = _str.length();

				//out.reserve(len);
				for (int i = 0; i < len; ++i) {
					ch.set(_str[i], s.f, s.color);
					out.append(1, ch);
				}

				return out;
			}

			fstr simple_bbcode(std::string str, style s) {
				return simple_bbcode(to_wstring(str), s);
			}

			fstr simple_bbcode(std::wstring _str, style s) {
				std::vector<int> to_skip;
				to_skip.resize(_str.size());

				fstr out;

				formatted_char ch;
				ch.font_used = s.f;
				int len = _str.length();

				//out.reserve(len);
				for (int i = 0; i < len; ++i) {
					ch.set(_str[i], s.f, s.color);
					out.append(1, ch);
				}

				auto opening_bracket_of_first = _str.find(L"[color=");

				while (opening_bracket_of_first != std::wstring::npos) {
					const auto opening_bracket_of_second = _str.find(L"[/color]", opening_bracket_of_first);
					const auto closing_bracket_of_second = opening_bracket_of_second + 8;

					const auto first_letter_of_argument = opening_bracket_of_first + 7;
					const auto closing_bracket_of_first = _str.find(L']', first_letter_of_argument);

					const auto argument = _str.substr(first_letter_of_argument, closing_bracket_of_first - first_letter_of_argument);

					style newstyle = s;

					if (argument == L"vsyellow") {
						newstyle.color = vsyellow;
					}
					if (argument == L"vsblue") {
						newstyle.color = vsblue;
					}
					if (argument == L"vscyan") {
						newstyle.color = vscyan;
					}
					if (argument == L"cyan") {
						newstyle.color = cyan;
					}
					if (argument == L"vsgreen") {
						newstyle.color = vsgreen;
					}
					if (argument == L"green") {
						newstyle.color = green;
					}
					if (argument == L"vsdarkgray") {
						newstyle.color = vsdarkgray;
					}
					if (argument == L"vslightgray") {
						newstyle.color = vslightgray;
					}
					if (argument == L"violet") {
						newstyle.color = violet;
					}
					if (argument == L"red") {
						newstyle.color = red;
					}
					if (argument == L"yellow") {
						newstyle.color = yellow;
					}
					if (argument == L"white") {
						newstyle.color = white;
					}
					if (argument == L"orange") {
						newstyle.color = orange;
					}
					if (argument == L"turquoise") {
						newstyle.color = turquoise;
					}

					for (size_t i = opening_bracket_of_first; i < closing_bracket_of_first+1; ++i) {
						to_skip[i] = 1;
					}

					for (size_t i = opening_bracket_of_second; i < closing_bracket_of_second; ++i) {
						to_skip[i] = 1;
					}

					for (size_t c = closing_bracket_of_first + 1; c < opening_bracket_of_second; ++c) {
						out[c].set(newstyle.f, newstyle.color);
					}

					opening_bracket_of_first = _str.find(L"[color=", closing_bracket_of_second);
				}

				size_t idx = 0;

				out.erase(std::remove_if(out.begin(), out.end(), [&to_skip, &idx](auto) {return to_skip[idx++]; }), out.end());

				return out;
			}

			void formatted_char::set(wchar_t ch, assets::font_id f, const rgba& p) {
				font_used = f;
				c = ch;
				memcpy(&r, &p, sizeof(rgba));
			}

			void formatted_char::set(assets::font_id f, const rgba& p) {
				font_used = f;
				memcpy(&r, &p, sizeof(rgba));
			}

			bool formatted_char::operator==(const formatted_char& b) const {
				return font_used == b.font_used && c == b.c;
			}

			style::style(assets::font_id f, rgba c) : f(f), color(c) {}

			style::style(const formatted_char& c) : f(c.font_used), color(rgba(c.r, c.g, c.b, c.a)) {}

			style::operator formatted_char() {
				formatted_char c;
				c.set(f, color);
				return c;
			}
		}
	}
}
