#pragma once
#include "augs/templates/snapshotted_player.h"

#define LOG_PLAYER 1

template <class... Args>
void PLR_LOG(Args&&... args) {
#if LOG_PLAYER
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_PLAYER
#define PLR_LOG_NVPS LOG_NVPS
#else
#define PLR_LOG_NVPS PLR_LOG
#endif

namespace augs {
	template <class A, class B>
	auto snapshotted_player<A, B>::get_current_step() const {
		return current_step;
	}

	template <class A, class B>
	auto snapshotted_player<A, B>::get_total_steps() const {
		if (step_to_entropy.size() > 0) {
			return std::max(current_step, (*step_to_entropy.rbegin()).first);
		}

		return current_step;
	}

	template <class A, class B>
	void snapshotted_player<A, B>::begin_replaying() {
		advance_mode = advance_type::REPLAYING;
	}

	template <class A, class B>
	void snapshotted_player<A, B>::begin_recording() {
		advance_mode = advance_type::RECORDING;
	}

	template <class A, class B>
	const fixed_delta_timer& snapshotted_player<A, B>::get_timer() const { 
		return timer;
	}

	template <class A, class B>
   	void snapshotted_player<A, B>::finish() {
		step_to_entropy.clear();
		current_step = 0;

		pause();
	}

	template <class A, class B>
   	void snapshotted_player<A, B>::pause() {
		advance_mode = advance_type::PAUSED;
	}

	template <class A, class B>
   	bool snapshotted_player<A, B>::is_paused() const {
		return advance_mode == advance_type::PAUSED;
	}

	template <class A, class B>
   	void snapshotted_player<A, B>::request_steps(const int amount) {
		additional_steps += amount;
	}

	template <class A, class B>
	double snapshotted_player<A, B>::get_speed() const {
		return is_paused() ? 0.0 : speed;
	}

	template <class A, class B>
   	void snapshotted_player<A, B>::set_speed(const double new_speed) {
		speed = new_speed;
	}

	template <class A, class B>
	template <class I, class SetSnapshot>
	void snapshotted_player<A, B>::seek_to(
		typename snapshotted_player<A, B>::step_type seeked_step,
		const I& input,
		SetSnapshot&& set_snapshot
	) {
		auto set = [&](const auto i) {
			PLR_LOG("Set snapshot num %x (size: %x)", i, snapshots.size());

			set_snapshot(i, snapshots.at(i));
			current_step = i * snapshot_frequency_in_steps;

			PLR_LOG("New current step: %x", current_step);
		};

		const auto seeked_adj_snapshot = std::min(
			static_cast<unsigned>(snapshots.size() - 1),
			seeked_step / snapshot_frequency_in_steps
		);

		PLR_LOG("Seeking from %x to %x", current_step, seeked_step);
		PLR_LOG("Adjacent snapshot: #%x", seeked_adj_snapshot);

		if (seeked_step < current_step) {
			PLR_LOG("Seek backwards");
			set(seeked_adj_snapshot);
		}

		/* At this point the current step is <= than the target step. */

		{
			/* Maybe we can speed up the forward seek by setting some future snapshot. */

			const auto step_of_adj_snapshot = seeked_adj_snapshot * snapshot_frequency_in_steps;

			const auto distance_from_adj_snapshot = seeked_step - step_of_adj_snapshot;
			const auto distance_from_seeked_step = seeked_step - current_step;

			PLR_LOG_NVPS(
				step_of_adj_snapshot,
				distance_from_adj_snapshot,
				distance_from_seeked_step
			);

			if (distance_from_adj_snapshot < distance_from_seeked_step) {
				PLR_LOG("Speedup the forward seek");

				set(seeked_adj_snapshot);
			}
		}

		/* Advance how much is needed from the time of the set snapshot. */

		PLR_LOG("Advancing the rest from %x to %x.", current_step, seeked_step);

		while (current_step < seeked_step) {
			advance_single_step(input);
		}
	}

	template <class A, class B>
	template <class MakeSnapshot>
	void snapshotted_player<A, B>::push_snapshot_if_needed(MakeSnapshot&& make_snapshot) {
		if (current_step % snapshot_frequency_in_steps == 0) {
			const auto snapshot_index = current_step / snapshot_frequency_in_steps;

			const bool valid_snapshot_exists = snapshot_index < snapshots.size();

			if (!valid_snapshot_exists) {
				ensure_eq(snapshot_index, snapshots.size());

				PLR_LOG("Snapshot step: %x. Snapshot #%x pushed.", current_step, snapshot_index);
				snapshots.emplace_back(make_snapshot(snapshot_index));
			}
			else {
				PLR_LOG("Snapshot step: %x. Snapshot #%x exists.", current_step, snapshot_index);
			}
		}
	}

	template <class entropy_type, class B>
	template <class I>
	void snapshotted_player<entropy_type, B>::advance_single_step(const I& in) {
		auto& step_i = current_step;
		auto& applied_entropy = in.total_collected;

		push_snapshot_if_needed(in.make_snapshot);

		{
			const auto next_snapshot_index = 1 + step_i / snapshot_frequency_in_steps;
			const bool outdated_snapshots_to_delete_exist = next_snapshot_index < snapshots.size();

			if (outdated_snapshots_to_delete_exist) {
				snapshots.erase(
					snapshots.begin() + next_snapshot_index,
					snapshots.end()
				);
			}
		}
		
		switch (advance_mode) {
			case advance_type::REPLAYING:
				if (const auto found_entropy = mapped_or_nullptr(step_to_entropy, step_i)) {
					applied_entropy = *found_entropy;
				}
				
			case advance_type::RECORDING:
				if (!applied_entropy.empty()) {
					step_to_entropy[step_i] = applied_entropy;
				}

			default: break;
		}

		in.step(applied_entropy);
		applied_entropy.clear();
		++step_i;
	}

	template <class entropy_type, class B>
	template <class I>
	int snapshotted_player<entropy_type, B>::advance(
		const I& in,
		delta frame_delta, 
		const delta& fixed_delta
	) {
		auto steps = additional_steps;

		if (!is_paused()) {
			timer.advance(frame_delta *= speed);
			steps += timer.extract_num_of_logic_steps(fixed_delta);
		}

		const auto performed_steps = steps;

		while (steps--) {
			advance_single_step(in);
		}

		additional_steps = 0;

		return performed_steps;
	}

	template <class A, class B>
	void snapshotted_player<A, B>::clear_later_entropies() {
		auto& ste = step_to_entropy;

		if (const auto it = ste.lower_bound(get_current_step()); it != ste.end()) {
			ste.erase(it, ste.end());
		}
	}
}
