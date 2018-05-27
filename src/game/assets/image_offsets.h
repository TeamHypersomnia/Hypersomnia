#pragma once
#include "augs/math/vec2.h"

struct torso_offets {
	// GEN INTROSPECTOR struct torso_offets
	vec2i primary_hand;
	vec2i secondary_hand;
	vec2i back;
	vec2i head;
	// END GEN INTROSPECTOR
};

struct gun_offsets {
	// GEN INTROSPECTOR struct gun_offsets
	vec2i bullet_spawn;
	// END GEN INTROSPECTOR
};

struct legs_offsets {
	// GEN INTROSPECTOR struct legs_offsets
	vec2i foot;
	// END GEN INTROSPECTOR
};

struct item_offsets {
	// GEN INTROSPECTOR struct item_offsets
	vec2i hand_anchor;
	vec2i back_anchor;
	// END GEN INTROSPECTOR
};

struct all_image_offsets {
	// GEN INTROSPECTOR struct all_image_offsets
	torso_offets torso;
	legs_offsets legs;
	gun_offsets gun;
	item_offsets item;
	// END GEN INTROSPECTOR
};

