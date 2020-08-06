#pragma once

struct hud_message_settings {
	// GEN INTROSPECTOR struct hud_message_settings
	float offset_mult = 0.6f;
	rgba text_color = white;
	float message_lifetime_secs = 5.0f;
	float message_fading_secs = 0.5f;
	int max_simultaneous_messages = 4;

	rgba background_color = rgba(0, 0, 0, 0);
	rgba background_border_color = rgba(0, 0, 0, 0);

	vec2 box_padding = vec2(32, 4);
	float box_separation = 1.f;
	// END GEN INTROSPECTOR
};

