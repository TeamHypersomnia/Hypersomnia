#pragma once
#include "augs/templates/snapshotted_player.h"
#include "augs/readwrite/byte_file.h"

#define LOG_PLAYER 1

#if LOG_PLAYER
#include "augs/log.h"
#endif

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
	template <class S>
	void snapshotted_player<A, B>::read_live_entropies(S& from) {
		read_map_until_eof(from, step_to_entropy);
	}

	template <class A, class B>
	auto snapshotted_player<A, B>::get_current_step() const {
		return current_step;
	}

	template <class A, class B>
	auto snapshotted_player<A, B>::get_total_steps() const {
		using step_type = typename snapshotted_player<A, B>::step_type;

		if (is_recording()) { 
			if (step_to_entropy.size() > 0) {
				return std::max(current_step, (*step_to_entropy.rbegin()).first);
			}

			return current_step;
		}
		else {
			if (step_to_entropy.size() > 0) {
				return (*step_to_entropy.rbegin()).first;
			}

			return static_cast<step_type>(0);
		}
	}

	template <class A, class B>
	void snapshotted_player<A, B>::begin_replaying() {
		advance_mode = advance_type::REPLAYING;
		PLR_LOG("begin_replaying at step: %x", current_step);
	}

	template <class A, class B>
	void snapshotted_player<A, B>::begin_recording() {
		advance_mode = advance_type::RECORDING;
		PLR_LOG("begin_recording at step: %x", current_step);
	}

	template <class A, class B>
	const fixed_delta_timer& snapshotted_player<A, B>::get_timer() const { 
		return timer;
	}

	template <class A, class B>
	const typename snapshotted_player<A, B>::snapshots_type& snapshotted_player<A, B>::get_snapshots() const { 
		return snapshots;
	}

	template <class A, class B>
   	void snapshotted_player<A, B>::finish() {
		step_to_entropy.clear();
		current_step = 0;
		additional_steps = 0;
		snapshots.clear();

		pause();
	}

	template <class A, class B>
   	void snapshotted_player<A, B>::pause() {
		advance_mode = advance_type::PAUSED;
		PLR_LOG("pause");
	}

	template <class A, class B>
   	bool snapshotted_player<A, B>::is_paused() const {
		return advance_mode == advance_type::PAUSED;
	}

	template <class A, class B>
	bool snapshotted_player<A, B>::is_recording() const {
		return advance_mode == advance_type::RECORDING;
	}

	template <class A, class B>
	bool snapshotted_player<A, B>::is_replaying() const {
		return advance_mode == advance_type::REPLAYING;
	}

	template <class A, class B>
	void snapshotted_player<A, B>::request_steps(typename snapshotted_player<A, B>::step_type amount) {
		additional_steps += amount;
	}

	template <class A, class B>
	double snapshotted_player<A, B>::get_speed() const {
		return is_paused() ? 0.0 : speed;
	}

	template <class A, class B>
	double snapshotted_player<A, B>::get_requested_speed() const {
		return speed;
	}

	template <class A, class B>
   	void snapshotted_player<A, B>::set_speed(const double new_speed) {
		speed = new_speed;
	}

	template <class A, class B>
	template <class I, class LoadSnapshot>
	void snapshotted_player<A, B>::seek_to(
		typename snapshotted_player<A, B>::step_type seeked_step,
		const I& input,
		LoadSnapshot&& load_snapshot
	) {
		if (seeked_step == current_step) {
			return;
		}

		PLR_LOG("Seeking from %x to %x", current_step, seeked_step);

		const auto& seeked_adj_snapshot = *std::prev(snapshots.upper_bound(seeked_step)); 
		const auto step_of_adj_snapshot = seeked_adj_snapshot.first;
	   
		auto seek_to_snapshot = [&]() {
			PLR_LOG("Set snapshot at step %x (size: %x)", current_step, snapshots.size());

			load_snapshot(step_of_adj_snapshot, seeked_adj_snapshot.second);

			current_step = step_of_adj_snapshot;
		};

		PLR_LOG("Adjacent snapshot step: #%x", step_of_adj_snapshot);

		if (seeked_step < current_step) {
			PLR_LOG("Seek backwards");
			seek_to_snapshot();
		}
		else {
			/* We're seeking forward. Setting the snapshot here may speed up the seek. */
			const auto distance_from_adj_snapshot = seeked_step - step_of_adj_snapshot;
			const auto distance_from_seeked_step = seeked_step - current_step;

			PLR_LOG_NVPS(
				distance_from_adj_snapshot,
				distance_from_seeked_step
			);

			if (distance_from_adj_snapshot < distance_from_seeked_step) {
				PLR_LOG("Speedup the forward seek");

				seek_to_snapshot();
			}
		}

		/* Advance how much is needed from the time of the set snapshot. */

		PLR_LOG("Advancing the rest from %x to %x.", current_step, seeked_step);

		while (current_step < seeked_step) {
			advance_single_step(input);
		}
	}

	template <class A, class B>
	template <class GenerateSnapshot>
	void snapshotted_player<A, B>::push_snapshot_if_needed(GenerateSnapshot&& generate_snapshot, const unsigned interval_in_steps) {
		if (is_recording() || (is_replaying() && get_current_step() == 0)) {
			const bool is_snapshot_time = [&]() {
				if (snapshots.empty()) {
					ensure_eq(static_cast<step_type>(0), current_step);
					return true;
				}

				if (interval_in_steps == 0) {
					return false;
				}

				const auto& it = *std::prev(snapshots.upper_bound(current_step));

				const auto since_last = current_step - it.first;
				return since_last >= interval_in_steps;
			}();

			if (is_snapshot_time) {
				PLR_LOG("Snapshot step: %x. Pushed.", current_step);
				snapshots[current_step] = generate_snapshot(current_step);
			}
		}
		else {
			const bool valid_snapshot_exists = found_in(snapshots, current_step);

			if (valid_snapshot_exists) {
				PLR_LOG("Snapshot step: %x. Exists.", current_step);

				generate_snapshot(std::nullopt);
			}
		}
	}

	template <class entropy_type, class B>
	template <class I>
	void snapshotted_player<entropy_type, B>::advance_single_step(const I& in) {
		push_snapshot_if_needed(in.generate_snapshot, in.settings.snapshot_interval_in_steps);

		auto considered_mode = advance_mode;

		if (considered_mode == advance_type::PAUSED) {
			/* If we're paused, treat is as replaying since we're probably seeking without applying any new inputs */
			considered_mode = advance_type::REPLAYING;
		}

		auto applied = entropy_type();

		switch (considered_mode) {
			case advance_type::REPLAYING:
				applied = mapped_or_default(step_to_entropy, current_step);
				break;
				
			case advance_type::RECORDING:
				{
					auto& entry = step_to_entropy[current_step];
					in.record_entropy(entry);
					applied = entry;

					if (entry.empty()) {
						step_to_entropy.erase(current_step);
					}
				}

				snapshots.erase(
					snapshots.upper_bound(current_step),
					snapshots.end()
				);
		
				break;

			default: break;
		}

		in.step(applied);
		++current_step;
	}

	template <class entropy_type, class B>
	template <class I>
	int snapshotted_player<entropy_type, B>::advance(
		const I& in,
		delta frame_delta, 
		const double inv_tickrate
	) {
		auto steps = additional_steps;

		if (additional_steps) {
			PLR_LOG_NVPS(additional_steps);
		}

		if (!is_paused()) {
			timer.advance(frame_delta *= speed);
			timer.max_steps_to_perform_at_once = 30;
			timer.mode = lag_spike_handling_type::CATCH_UP;
			steps += timer.extract_num_of_logic_steps(inv_tickrate);
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

	template <class A, class B>
	template <class F>
	void snapshotted_player<A, B>::for_each_later_entropy(F&& callback) {
		auto& ste = step_to_entropy;
		auto it = ste.lower_bound(get_current_step());

		while (it != ste.end()) {
			auto& entry = *it;
			callback(entry);
			++it;
		}
	}
}
