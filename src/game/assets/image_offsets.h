#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"

struct torso_offsets {
	// GEN INTROSPECTOR struct torso_offsets
	transformi primary_hand;
	transformi secondary_hand;
	transformi back;
	transformi head;
	transformi legs;
	// END GEN INTROSPECTOR

	void flip_vertically() {
		primary_hand.flip_vertically();
		secondary_hand.flip_vertically();
		back.flip_vertically();
		head.flip_vertically();

		std::swap(primary_hand, secondary_hand);
	}
};

struct gun_offsets {
	// GEN INTROSPECTOR struct gun_offsets
	vec2i bullet_spawn;
	transformi detachable_magazine;
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
	vec2i head_anchor;
	vec2i attachment_anchor;
	vec2i beep_offset;
	// END GEN INTROSPECTOR
};

struct all_image_offsets {
	// GEN INTROSPECTOR struct all_image_offsets
	torso_offsets torso;
	legs_offsets legs;
	gun_offsets gun;
	item_offsets item;
	// END GEN INTROSPECTOR
};

