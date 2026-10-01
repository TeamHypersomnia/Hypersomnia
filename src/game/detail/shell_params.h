#pragma once
#include "augs/math/declare_math.h"

/*
	All the parameters of shells ejected from guns - thrown up like explosives, only higher.
	Which casings, sounds and pitches every gun's shells get is up to their flavours - see make_shell in test_scenes/ingredients/guns.cpp.

	Their heights are rolled within the gun's shell_height - the higher, the longer and the more times they hop,
	until a hop would be lower than SHELL_LAST_HOP_HEIGHT, and the louder they hit the floor,
	as the roll picks the variation of their sounds.
	Their hops vary, and every hop is as high as its duration would physically take - heights go with durations squared.

	Like dropped items, they mostly stop spinning as they first hit the floor - and every hit spins them anew,
	by SHELL_HIT_SPIN_IMPULSE_MIN to MAX degrees per second, either way - as the speed of the impact,
	with the square root of the height of the hop that ends, fully from SHELL_HIT_SPIN_FULL_AT_HEIGHT.
	Every hit pushes those that roll sideways, and the others slightly on along their motion -
	and after the last one the rolling ones roll off sideways for a moment. They roll to whichever side is closer to where they fly or are kicked.
	Characters don't touch them until they first hit the floor, and then don't toss them around - only kick them now and then:
	out of SHELL_KICK_OUTCOMES kicks, SHELL_KICK_PASSES_THROUGH don't touch them, SHELL_KICK_ROLLS roll them off sideways, harder,
	and the rest nudge them away. Other bodies they collide with as usual, and items - see SHELL_ITEM_CONTACT_ROLL_CHANCE.

	The sideways push eases with the height of the hop that ends: as the speed of the impact, with its square root -
	the SHELL_LOW_HOP_ROLL speeds at SHELL_LOW_HOP_ROLL_AT_HEIGHT, at most SHELL_LOW_HOP_ROLL_MAX_MULT of them.
*/

/*
	Longer shells spin slower, so that their ends move about as fast as those of a SHELL_SPIN_REFERENCE_LENGTH px long one -
	both as they're ejected and as they hit the floor. Shorter ones spin as the reference one.
	Fast ones spin SHELL_FAST_SPIN_CHANCE of the time - see invariants::gun::shell_fast_angular_velocity.
*/

inline constexpr real32 SHELL_SPIN_REFERENCE_LENGTH = 13.f;
inline constexpr real32 SHELL_FAST_SPIN_CHANCE = 0.3f;

inline constexpr real32 SHELL_HOP_SECS_AT_UNIT_HEIGHT = 0.16f;
inline constexpr real32 SHELL_MIN_HOP_SECS = 0.05f;
inline constexpr real32 SHELL_LAST_HOP_HEIGHT = 0.04f;
inline constexpr real32 SHELL_HOP_DURATION_VARIATION = 0.2f;

inline constexpr real32 SHELL_SPIN_KEPT = 0.2f;
inline constexpr real32 SHELL_HIT_SPIN_IMPULSE_MIN = 300.f;
inline constexpr real32 SHELL_HIT_SPIN_IMPULSE_MAX = 1200.f;
inline constexpr real32 SHELL_HIT_SPIN_FULL_AT_HEIGHT = 2.f;

inline constexpr real32 SHELL_PUSH_MIN_SPEED = 15.f;
inline constexpr real32 SHELL_PUSH_MAX_SPEED = 40.f;
inline constexpr real32 SHELL_LOW_HOP_ROLL_MIN_SPEED = 10.f;
inline constexpr real32 SHELL_LOW_HOP_ROLL_MAX_SPEED = 30.f;
inline constexpr real32 SHELL_LOW_HOP_ROLL_AT_HEIGHT = 0.6f;
inline constexpr real32 SHELL_LOW_HOP_ROLL_MAX_MULT = 2.f;

inline constexpr real32 SHELL_ROLL_MIN_SPEED = 60.f;
inline constexpr real32 SHELL_ROLL_MAX_SPEED = 160.f;

/*
	Rolling off after the last hit keeps this much of the motion along the shell's axis -
	it slides on as it rolls, until damping stops the slide, instead of abruptly changing course.
*/

inline constexpr real32 SHELL_ROLL_KEPT_SLIDE = 0.4f;

inline constexpr real32 SHELL_KICK_ROLL_MIN_SPEED = 220.f;
inline constexpr real32 SHELL_KICK_ROLL_MAX_SPEED = 380.f;
inline constexpr real32 SHELL_NUDGE_MIN_SPEED = 30.f;
inline constexpr real32 SHELL_NUDGE_MAX_SPEED = 90.f;
inline constexpr real32 SHELL_KICK_COOLDOWN_MS = 1000.f;

/*
	Items never pass through shells - touching one, a shell either bounces off it, or rolls off it as if kicked,
	SHELL_ITEM_CONTACT_ROLL_CHANCE of the time.
*/

inline constexpr real32 SHELL_ITEM_CONTACT_ROLL_CHANCE = 0.5f;

inline constexpr int SHELL_KICK_OUTCOMES = 6;
inline constexpr int SHELL_KICK_PASSES_THROUGH = 2;
inline constexpr int SHELL_KICK_ROLLS = 3;

inline constexpr real32 SHELL_FALL_PITCH_VARIATION = 0.05f;
inline constexpr real32 SHELL_FLOOR_HIT_PITCH_VARIATION = 0.03f;

/*
	Smoke of bigger calibers - by ejection_smoke_size_mult of their casings - is bigger by these parts of how much bigger it is:
	its particles and how long it lasts - e.g. 1.8x as big smoke has 1.48x as big particles, lasting 1.8x as long.
*/

inline constexpr float SHELL_SMOKE_PARTICLE_SIZE_PER_SIZE = 0.6f;
inline constexpr float SHELL_SMOKE_DURATION_PER_SIZE = 1.0f;
