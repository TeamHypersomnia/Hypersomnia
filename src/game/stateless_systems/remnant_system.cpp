#include <algorithm>
#include <vector>
#include "game/messages/queue_deletion.h"
#include "game/stateless_systems/remnant_system.h"

#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/inventory/item_falling.h"
#include "game/components/rigid_body_component.h"

void remnant_system::advance_falling_remnants(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto now = cosm.get_timestamp();
	const auto dt = cosm.get_fixed_delta();

	cosm.for_each_having<components::remnant>(
		[&](const auto subject) {
			auto& state = subject.template get<components::remnant>();
			auto& fall = state.fall;

			/*
				Most remnants lie still - skip them right away.
			*/

			const bool kicked = !state.pending_kick.is_zero();
			const bool floor_hit_due = ::is_floor_hit_due(fall, now, dt);

			if (!kicked && !floor_hit_due) {
				return;
			}

			const auto body = subject.template get<components::rigid_body>();

			const auto sideways = vec2::from_degrees(subject.get_logic_transform().rotation).perpendicular_cw();

			/*
				Rolling, it must not spin at all, or it would look off.
			*/

			auto roll = [&](const real32 side, const real32 speed) {
				body.set_velocity(sideways * side * speed);
				body.set_angular_velocity(0.f);
			};

			const auto& def = subject.template get<invariants::remnant>();

			auto play_roll_sound = [&]() {
				if (!def.roll_sound.id.is_set()) {
					return;
				}

				auto start = sound_effect_start_input::fire_and_forget(subject.get_logic_transform());
				start.shell_ejected_by = state.ejected_by;
				start.shell_roll = true;
				start.variation_number = augs::hash_multiple(fall.seed, state.num_kicks);

				def.roll_sound.start(step, start, predictable_only_by(state.kicked_by));
			};

			/*
				Rolls to whichever side is closer to the direction - or a random one if it's perpendicular.
			*/

			auto side_towards = [&](const vec2 direction, randomization& rng) {
				const auto dot = sideways.dot(direction);

				if (dot == 0.f) {
					return ::random_sign(rng);
				}

				return dot > 0.f ? 1.f : -1.f;
			};

			if (kicked) {
				auto rng = randomization(augs::hash_multiple(fall.seed, state.num_kicks++));

				/*
					Kicks by items roll the shells for sure - see SHELL_ITEM_CONTACT_ROLL_CHANCE.
				*/

				const auto outcome = rng.randval(0, SHELL_KICK_OUTCOMES - 1);
				const bool passes_through = !state.pending_kick_rolls && outcome < SHELL_KICK_PASSES_THROUGH;
				const bool rolled = def.rolls && !passes_through && (state.pending_kick_rolls || outcome < SHELL_KICK_PASSES_THROUGH + SHELL_KICK_ROLLS);
				const bool nudged = !passes_through && !rolled;

				if (rolled) {
					const auto side = side_towards(state.pending_kick, rng);
					roll(side, rng.randval(SHELL_KICK_ROLL_MIN_SPEED, SHELL_KICK_ROLL_MAX_SPEED));
					play_roll_sound();
				}
				else if (nudged) {
					body.set_velocity(state.pending_kick * rng.randval(SHELL_NUDGE_MIN_SPEED, SHELL_NUDGE_MAX_SPEED));
				}

				state.pending_kick = vec2::zero;
				state.pending_kick_rolls = false;
			}

			if (!floor_hit_due) {
				return;
			}

			/*
				Shells fall only once - see start_shell_falling.
			*/

			const auto fall_seed = fall.seed;
			const auto hit_index = fall.floor_hits_done;
			const auto hop_height = fall.hop_height;

			::start_shell_like_floor_hit(step, subject, body, fall, def.floor_hit_sounds, state.ejected_by);
			::count_floor_hit_and_start_next_hop(fall, SHELL_HOP_DURATION_VARIATION, SHELL_MIN_HOP_SECS, now, cosm.get_clock().logic_speed);
			fall.hop_height = ::calc_shell_hop_height(fall.hop_duration_secs);

			auto side_rng = ::make_floor_hit_rng(fall_seed, hit_index, floor_hit_rng_purpose::ROLL_SIDE);
			const auto roll_side = side_towards(body.get_velocity(), side_rng);

			auto push_rng = ::make_floor_hit_rng(fall_seed, hit_index, floor_hit_rng_purpose::PUSH);

			if (fall.floor_hits_left == 0) {
				if (def.rolls) {
					const auto speed = push_rng.randval(SHELL_ROLL_MIN_SPEED, SHELL_ROLL_MAX_SPEED);
					const auto axis = vec2::from_degrees(subject.get_logic_transform().rotation);
					const auto slide = axis * axis.dot(body.get_velocity()) * SHELL_ROLL_KEPT_SLIDE;

					roll(roll_side, speed);
					body.set_velocity(body.get_velocity() + slide);
				}
			}
			else if (def.rolls) {
				const auto speed = push_rng.randval(SHELL_LOW_HOP_ROLL_MIN_SPEED, SHELL_LOW_HOP_ROLL_MAX_SPEED) * ::calc_shell_low_hop_roll_mult(hop_height);
				body.set_velocity(body.get_velocity() + sideways * roll_side * speed);
			}
			else {
				::push_on_shell_floor_hit(body, push_rng);
			}
		}
	);
}

void remnant_system::shrink_and_destroy_remnants(const logic_step step) const {
	auto& cosm = step.get_cosmos();

	/*
		Remnants are decorative, so they last as long regardless of logic speed.
	*/

	const auto clk = cosm.get_clock().get_real_clock();

	std::size_t num_kept = 0;
	std::size_t num_kept_not_evicted = 0;

	entity_id oldest_not_evicted;
	auto oldest_not_evicted_born = augs::stepped_timestamp();

	cosm.for_each_having<components::remnant>(
		[&](const auto subject) {
			const auto& def = subject.template get<invariants::remnant>();
			auto& state = subject.template get<components::remnant>();

			auto shrink_or_delete = [&](const real32 remaining_ms) {
				const auto size_mult = def.start_shrinking_when_remaining_ms > 0.f ? remaining_ms / def.start_shrinking_when_remaining_ms : remaining_ms;

				if (size_mult <= 0.f) {
					step.queue_deletion_of(subject, "Remnant expiration");
				}
				else if (size_mult < 1.f) {
					state.last_size_mult = size_mult;
				}
			};

			if (!def.kept_until_evicted) {
				shrink_or_delete(clk.get_remaining_ms(def.lifetime_secs * 1000, subject.when_born()));
				return;
			}

			++num_kept;

			if (state.when_evicted.was_set()) {
				shrink_or_delete(clk.get_remaining_ms(def.start_shrinking_when_remaining_ms, state.when_evicted));
				return;
			}

			++num_kept_not_evicted;

			const auto born = subject.when_born();

			if (!oldest_not_evicted_born.was_set() || born < oldest_not_evicted_born) {
				oldest_not_evicted_born = born;
				oldest_not_evicted = subject.get_id();
			}
		}
	);

	/*
		Remnants kept until evicted start shrinking one by one once there are too many -
		the oldest one each step, to spread the work.
	*/

	if (num_kept_not_evicted > MAX_KEPT_REMNANTS) {
		if (const auto oldest = cosm[oldest_not_evicted]) {
			oldest.template dispatch_on_having_all<components::remnant>([&](const auto& typed_oldest) {
				typed_oldest.template get<components::remnant>().when_evicted = clk.now;
			});
		}
	}

	/*
		Past the hard limit - e.g. when many are spawned at once - the oldest ones are deleted right away.
	*/

	if (num_kept > MAX_KEPT_REMNANTS_HARD_LIMIT) {
		std::vector<std::pair<augs::stepped_timestamp, entity_id>> kept;
		kept.reserve(num_kept);

		cosm.for_each_having<components::remnant>(
			[&](const auto subject) {
				if (subject.template get<invariants::remnant>().kept_until_evicted) {
					kept.emplace_back(subject.when_born(), subject.get_id());
				}
			}
		);

		const auto num_deleted = kept.size() - MAX_KEPT_REMNANTS_HARD_LIMIT;

		std::nth_element(
			kept.begin(),
			kept.begin() + num_deleted,
			kept.end(),
			[](const auto& a, const auto& b) {
				return a.first < b.first || (a.first == b.first && a.second < b.second);
			}
		);

		for (std::size_t i = 0; i < num_deleted; ++i) {
			step.queue_deletion_of(kept[i].second, "Remnant over the hard limit");
		}
	}
}
