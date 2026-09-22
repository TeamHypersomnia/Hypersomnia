#pragma once
#include <optional>

enum class interpolation_mode {
	// GEN INTROSPECTOR enum class interpolation_mode
	NONE,
	INTERPOLATE,
	EXTRAPOLATE,

	COUNT
	// END GEN INTROSPECTOR
};

/*
	Resolved per entity, for position and rotation separately.
	EXTRAPOLATE only pays off for whatever moves predictably.
*/
struct interpolation_modes {
	// GEN INTROSPECTOR struct interpolation_modes
	interpolation_mode controlled_character_position = interpolation_mode::INTERPOLATE;
	interpolation_mode controlled_character_rotation = interpolation_mode::INTERPOLATE;
	interpolation_mode other_characters_position = interpolation_mode::INTERPOLATE;
	interpolation_mode other_characters_rotation = interpolation_mode::INTERPOLATE;
	interpolation_mode bullets = interpolation_mode::INTERPOLATE;
	interpolation_mode everything_else = interpolation_mode::INTERPOLATE;
	// END GEN INTROSPECTOR

	bool operator==(const interpolation_modes& b) const = default;

	static auto all(const interpolation_mode mode) {
		interpolation_modes out;

		out.controlled_character_position = mode;
		out.controlled_character_rotation = mode;
		out.other_characters_position = mode;
		out.other_characters_rotation = mode;
		out.bullets = mode;
		out.everything_else = mode;

		return out;
	}

	void set_all(const interpolation_mode mode) {
		*this = all(mode);
	}

	/* Empty once the categories disagree, which the GUI shows as a custom setup. */
	std::optional<interpolation_mode> find_uniform() const {
		if (*this == all(controlled_character_position)) {
			return controlled_character_position;
		}

		return std::nullopt;
	}

	bool any_enabled() const {
		return *this != all(interpolation_mode::NONE);
	}
};

struct interpolation_settings {
	// GEN INTROSPECTOR struct interpolation_settings
	interpolation_modes modes;
	float misprediction_smoothing_speed = 1000.f;
	// END GEN INTROSPECTOR

	bool operator==(const interpolation_settings& b) const = default;

	bool enabled() const {
		return modes.any_enabled();
	}
};
