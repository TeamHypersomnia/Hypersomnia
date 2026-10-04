#pragma once
#include <algorithm>
#include <cstdint>
#include "3rdparty/Box2D/Common/b2Settings.h"
#include "augs/math/damped_motion.h"
#include "augs/misc/timing/stepped_timing.h"
#include "game/components/hand_fuse_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/movement_component.h"
#include "game/components/rigid_body_component.h"
#include "game/detail/crosshair_math.hpp"

/*
	How a held explosive is launched once released.

	Thrown with the primary button or with a throw key, it is aimed:
	it flies just as fast - and is damped just as much - as to stop by its fuse exactly at the crosshair, however far, if nothing is in the way
	and if it is not cooked - held armed for a while - in which case it explodes sooner, closer to the thrower.
	With sprint held, it flies AIMED_THROW_SPRINT_DISTANCE_MULT times as far, regardless of how fast the thrower moves.

	Released with the secondary button, it is lobbed nearby at a fixed speed, boosted by the motion of the thrower.

	Both the logic and the view's trajectory preview launch it through calc_explosive_throw_launch.
*/

/*
	A safety margin below the most a body can travel per step in Box2D - b2_maxTranslation -
	above which Box2D would slow it down, so that it would fall short.
*/

inline constexpr real32 AIMED_THROW_MAX_TRANSLATION_MULT = 0.99f;

struct explosive_throw_launch {
	real32 speed = 0.f;
	real32 angular_impulse = 0.f;
	real32 damping_mult = 1.f;
	real32 momentum_boost = 0.f;
};

struct explosive_movement_boost {
	real32 added_speed = 0.f;
	real32 damping_mult = 1.f;
};

/*
	How many physics steps a released explosive flies before its fuse detonates it.

	It is released at clk.now, before the detonation check and the physics step of that same step,
	so it flies for every step until the first one at which the fuse is ready.
	An explosive not armed yet is assumed to be armed and released right now.
*/

inline uint32_t calc_explosive_flight_steps(
	const augs::stepped_clock& clk,
	const real32 fuse_delay_ms,
	const augs::stepped_timestamp when_armed
) {
	constexpr uint32_t max_steps = 10000;

	const auto armed = when_armed.was_set() ? when_armed : clk.now;
	const auto dt_ms = clk.dt.in_milliseconds();

	/*
		Starts from an estimate a few steps below the answer,
		then finds it exactly with is_ready, just like the fuse logic checks it.
	*/

	uint32_t steps = [&]() {
		if (dt_ms <= 0.f) {
			return uint32_t(0);
		}

		const auto passed_steps = static_cast<real32>(clk.now.step - armed.step);
		const auto estimate = fuse_delay_ms / dt_ms - passed_steps - 2.f;

		return static_cast<uint32_t>(std::clamp(estimate, 0.f, static_cast<real32>(max_steps)));
	}();

	while (steps < max_steps) {
		const auto at = augs::stepped_clock::from_timestamp(clk, augs::stepped_timestamp { clk.now.step + steps });

		if (at.is_ready(fuse_delay_ms, armed)) {
			break;
		}

		++steps;
	}

	return steps;
}

/*
	Running forward while releasing with the secondary button throws farther.
*/

template <class H>
explosive_movement_boost calc_explosive_movement_boost(const H& holder) {
	auto result = explosive_movement_boost();

	const auto vel = holder.get_effective_velocity();
	const auto speed = vel.length();

	if (speed > 1.0f) {
		const auto look_dir = holder.get_logic_transform().get_direction();
		const auto vel_dir = vec2(vel).normalize_hint(speed);

		auto parallel_mult = (look_dir.dot(vel_dir) + 1) / 2;

		if (parallel_mult > 0.8f) {
			parallel_mult -= 0.8f;
			parallel_mult *= 5.0f;

			float dash_bonus = 0.0f;

			if (const auto movement = holder.template find<components::movement>()) {
				dash_bonus = movement->linear_inertia_ms;
			}

			const auto total_speed = dash_bonus * 8 + speed * 4;
			result.added_speed = total_speed * parallel_mult;

			const auto considered_max_speed = 10000.0f;
			const auto speed_mult = total_speed / considered_max_speed;

			const auto min_damping = 0.4f;

			result.damping_mult = augs::interp(1.0f, min_damping, speed_mult * parallel_mult);
			result.damping_mult = std::max(result.damping_mult, min_damping);
		}
	}

	return result;
}

inline constexpr real32 AIMED_THROW_SPRINT_DISTANCE_MULT = 2.f;

template <class H>
bool wants_aimed_throw_farther(const H& holder) {
	if (const auto movement = holder.template find<components::movement>()) {
		return movement->flags.sprinting;
	}

	return false;
}

struct aimed_explosive_throw {
	real32 speed = 0.f;
	real32 damping_mult = 1.f;
};

/*
	Reaches any distance:
	if even the most a body can travel per step in Box2D would not get it there in time,
	it flies at that speed, only damped less.
	The distance only falls as the damping grows, so the damping is found by bisection -
	in a fixed number of iterations, so that it stays deterministic.
*/

inline constexpr int AIMED_THROW_DAMPING_BISECTION_ITERATIONS = 32;

template <class E, class H>
aimed_explosive_throw calc_aimed_explosive_throw(const E& explosive, const H& holder) {
	const auto& cosm = explosive.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto dt_secs = clk.dt.in_seconds();

	const auto& fuse = explosive.template get<components::hand_fuse>();

	auto damping = explosive.template get<invariants::rigid_body>().damping;
	damping.sanitize();

	const auto linear_damping = damping.linear;

	/*
		As strong as if it was not held armed at all, so that cooking it makes it explode sooner - and closer.
	*/

	const auto steps = ::calc_explosive_flight_steps(clk, fuse.fuse_delay_ms, augs::stepped_timestamp());

	const auto crosshair_pos = holder.get_world_crosshair_transform().pos;
	const auto distance_mult = ::wants_aimed_throw_farther(holder) ? AIMED_THROW_SPRINT_DISTANCE_MULT : 1.f;
	const auto aimed_distance = (crosshair_pos - explosive.get_logic_transform().pos).length() * distance_mult;
	const auto speed = augs::calc_initial_speed_for_damped_distance(aimed_distance, linear_damping, dt_secs, steps);

	const auto max_speed = AIMED_THROW_MAX_TRANSLATION_MULT * cosm.get_si().get_pixels(b2_maxTranslation) / dt_secs;

	if (speed <= max_speed || linear_damping <= 0.f) {
		return { std::min(speed, max_speed), 1.f };
	}

	auto lower = 0.f;
	auto upper = linear_damping;

	for (int i = 0; i < AIMED_THROW_DAMPING_BISECTION_ITERATIONS; ++i) {
		const auto middle = (lower + upper) / 2;

		if (augs::calc_damped_distance(max_speed, middle, dt_secs, steps) > aimed_distance) {
			lower = middle;
		}
		else {
			upper = middle;
		}
	}

	return { max_speed, (lower + upper) / 2 / linear_damping };
}

template <class E, class H>
explosive_throw_launch calc_explosive_throw_launch(const E& explosive, const H& holder, const uint8_t arming_source) {
	const auto& fuse_def = explosive.template get<invariants::hand_fuse>();

	auto result = explosive_throw_launch();

	if (fuse_def.override_release_impulse) {
		/* True for the bomb so it never gets thrown. */
		result.speed = fuse_def.additional_release_impulse.linear;
		result.angular_impulse = fuse_def.additional_release_impulse.angular;

		return result;
	}

	const auto capability = holder.template find<invariants::item_slot_transfers>();

	if (capability == nullptr) {
		return result;
	}

	if (arming_source == arming_source_type::SHOOT_SECONDARY_INTENT) {
		const auto impulse = capability->standard_drop_impulse + fuse_def.additional_secondary_release_impulse;

		result.speed = impulse.linear;
		result.angular_impulse = impulse.angular;

		if (fuse_def.apply_movement_boost) {
			const auto boost = ::calc_explosive_movement_boost(holder);

			result.speed += boost.added_speed;
			result.damping_mult = boost.damping_mult;
			result.momentum_boost = boost.added_speed;
		}

		return result;
	}

	const auto impulse = capability->standard_throw_impulse + fuse_def.additional_release_impulse;

	const auto aimed = ::calc_aimed_explosive_throw(explosive, holder);

	result.speed = aimed.speed;
	result.damping_mult = aimed.damping_mult;
	result.angular_impulse = impulse.angular;

	return result;
}
