#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"

struct torso_offets {
	// GEN INTROSPECTOR struct torso_offets
	transformi primary_hand;
	transformi secondary_hand;
	transformi back;
	transformi head;
	// END GEN INTROSPECTOR
};

struct gun_offsets {
	// GEN INTROSPECTOR struct gun_offsets
	vec2i bullet_spawn;
	transformi magazine;
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
	vec2i attachment_anchor;
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

