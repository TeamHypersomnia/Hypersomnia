#pragma once
#include <vector>
#include "math/vec2d.h"
#include "graphics/pixel.h"
#include "texture_baker/font.h"
using namespace augs::texture_baker;

namespace augs {
	namespace graphics {
		namespace gui {
			namespace text {
				struct formatted_char {
					font* font_used;
					wchar_t c;
					unsigned char r, g, b, a;
					void set(wchar_t, font* = 0, const pixel_32& = pixel_32());
					void set(font* = 0, const pixel_32& = pixel_32());
				};

				struct style {
					font* f;
					pixel_32 color;
					style(font*, pixel_32);
					style(const formatted_char&);
					operator formatted_char();
				};

				typedef std::basic_string<formatted_char> fstr;
			}
				
			namespace text {
				extern fstr format(const std::wstring&, style);
				extern void format(const std::wstring&, style, fstr&);
				extern void format(const wchar_t*, style, fstr&);
				extern fstr format(const wchar_t*, style);
			}
		}
	}
	namespace misc {
		extern std::wstring wstr(const graphics::gui::text::fstr& f);
		template <class T>
		T wnum(const graphics::gui::text::fstr& f) {
			std::wistringstream ss(wstr(f));
			T v;
			ss >> v;
			return v;
		}
		
	}
}

