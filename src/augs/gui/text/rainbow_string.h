#pragma once
#include <cstddef>
#include <string>
#include "augs/gui/formatted_string.h"

namespace augs {
	namespace gui {
		namespace text {
			/*
				Every character in its own color of a wave going through the rainbow over time - see rgba::get_bright_wave -
				each a bit behind the previous one, so that the colors flow along the text.
				Keeps the alpha of the style.
			*/

			inline formatted_string rainbow_string(const std::string& str, const style s, const double secs) {
				constexpr auto wave_secs_per_char = 0.05;
				constexpr auto wave_slowdown = 4.0;

				auto result = formatted_string(str, s);

				for (std::size_t i = 0; i < result.size(); ++i) {
					auto col = rgba::get_bright_wave(secs / wave_slowdown - static_cast<double>(i) * wave_secs_per_char, 0.55);
					col.a = s.color.a;
					result[i].set_color(col);
				}

				return result;
			}
		}
	}
}
