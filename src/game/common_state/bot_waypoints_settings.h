#pragma once

/*
	Where the bots' push and patrol waypoints come from - see random_waypoints_generator.h.
*/

enum class bot_waypoints_mode {
	// GEN INTROSPECTOR enum class bot_waypoints_mode
	AUTOMATIC,
	YOURS_AND_GENERATED,
	RANDOM_EVERY_ROUND,

	COUNT
	// END GEN INTROSPECTOR
};

struct bot_waypoints_settings {
	// GEN INTROSPECTOR struct bot_waypoints_settings
	bot_waypoints_mode mode = bot_waypoints_mode::AUTOMATIC;
	float density = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const bot_waypoints_settings&) const = default;
};

/*
	The editor's preview of the generated waypoints - never one of the author's waypoints in game.
*/

inline constexpr const char* random_waypoints_layer_name_v = "random_waypoints";
