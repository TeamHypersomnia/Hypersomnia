#pragma once
#include "stdafx.h"

#include "system.h"
namespace augs {
	namespace misc {
		std::wstring wstr(const graphics::gui::text::fstr& f) {
			size_t l = f.size();
			std::wstring ww;
			ww.reserve(l);
			for(size_t i = 0; i < l; ++i)
				ww += f[i].c;

			return ww;
		}
	}

	namespace graphics {
		namespace gui {
			namespace text {
				void format(const wchar_t* _str, style s, fstr& out) {
					out.clear();
					formatted_char ch;
					int len = wcslen(_str);
					for(int i = 0; i < len; ++i) {
						ch.set(_str[i], s.f, s.color);
						out.push_back(ch);
					}
				}

				fstr format(const wchar_t* _str, style s) {
					fstr out;	

					formatted_char ch;
					ch.font_used = s.f;
					int len = wcslen(_str);

					//out.reserve(len);
					for(int i = 0; i < len; ++i) {
						ch.set(_str[i], s.f, s.color);
						out.push_back(ch);
					}

					return out;
				}

				void format(const std::wstring& _str, style s, fstr& out) {
					out.clear();
					formatted_char ch;
					int len = _str.length();
					for(int i = 0; i < len; ++i) {
						ch.set(_str[i], s.f, s.color);
						out.push_back(ch);
					}
				}

				fstr format(const std::wstring& _str, style s) {
					fstr out;	

					formatted_char ch;
					ch.font_used = s.f;
					int len = _str.length();

					//out.reserve(len);
					for(int i = 0; i < len; ++i) {
						ch.set(_str[i], s.f, s.color);
						out.push_back(ch);
					}

					return out;
				}
			}


			namespace text {
				void formatted_char::set(wchar_t ch, font* f, const augs::graphics::pixel_32& p) {
					font_used = f;
					c = ch;
					memcpy(&r, &p, sizeof(pixel_32));
				}

				void formatted_char::set(font* f, const pixel_32& p) {
					font_used = f;
					memcpy(&r, &p, sizeof(pixel_32));
				}

				style::style(font* f, pixel_32 c) : f(f), color(c) {}

				style::style(const formatted_char& c) : f(c.font_used), color(pixel_32(c.r, c.g, c.b, c.a)) {}

				style::operator formatted_char() {
					formatted_char c;
					c.set(f, color);
					return c;
				}
			}
		}
	}
}