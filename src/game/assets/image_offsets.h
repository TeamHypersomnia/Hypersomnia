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
	real32 strafe_facing_offset = 0.f;
	// END GEN INTROSPECTOR

	void flip_vertically() {
		primary_hand.flip_vertically();
		secondary_hand.flip_vertically();
		back.flip_vertically();
		head.flip_vertically();
		strafe_facing_offset *= -1;

		std::swap(primary_hand, secondary_hand);
	}
};

struct gun_offsets {
	// GEN INTROSPECTOR struct gun_offsets
	vec2i bullet_spawn;
	transformi detachable_magazine;
	transformi shell_spawn;
	// END GEN INTROSPECTOR
};

struct legs_offsets {
	// GEN INTROSPECTOR struct legs_offsets
	vec2i foot;
	// END GEN INTROSPECTOR
};

struct item_offsets {
	// GEN INTROSPECTOR struct item_offsets
	transformi hand_anchor;
	transformi back_anchor;
	transformi head_anchor;
	transformi attachment_anchor;
	transformi beep_offset;
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

