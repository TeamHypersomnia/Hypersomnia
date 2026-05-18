#pragma once
#include <array>
#include <string>
#include <vector>
#include <utility>

#include "augs/graphics/rgba.h"
#include "augs/graphics/neon_light_color.h"

namespace augs {
	inline const std::array<std::pair<rgba, rgba>, 6> gore_color_map = {{
		{ rgba(255, 0, 0, 255), rgba(0, 255, 0, 255) },
		{ rgba(207, 0, 0, 255), rgba(0, 207, 0, 255) },
		{ rgba(192, 0, 0, 255), rgba(0, 192, 0, 255) },
		{ rgba(128, 0, 0, 255), rgba(0, 128, 0, 255) },
		{ rgba(91,  0, 0, 255), rgba(0, 91,  0, 255) },
		{ rgba(31,  0, 0, 255), rgba(0, 31,  0, 255) }
	}};

	inline const std::vector<std::string> gore_sprites_match_words = {
		"gore",
		"blood",
		"tattered",
		"corpse"
	};

	inline bool path_matches_gore_words(const std::string& path) {
		for (const auto& word : gore_sprites_match_words) {
			if (path.find(word) != std::string::npos) {
				return true;
			}
		}

		return false;
	}

	inline bool try_remap_gore_color(rgba& c) {
		for (const auto& [from, to] : gore_color_map) {
			if (c == from) {
				c = to;
				return true;
			}
		}

		return false;
	}

	template <class Img>
	inline void remap_gore_pixels(Img& img) {
		for (auto& pixel : img) {
			::augs::try_remap_gore_color(pixel);
		}
	}

	inline void remap_gore_light_colors(std::vector<neon_light_color>& colors) {
		for (auto& c : colors) {
			::augs::try_remap_gore_color(c.color);
		}
	}
}
