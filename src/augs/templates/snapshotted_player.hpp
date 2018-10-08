#pragma once
#include "augs/templates/snapshotted_player.h"

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
		const auto previous_snapshot_index = std::min(
			static_cast<unsigned>(snapshots.size() - 1),
			seeked_step / snapshot_frequency_in_steps
		);

		auto set_ith = [&](const auto i) {
			set_snapshot(i, snapshots.at(i));
			current_step = i * snapshot_frequency_in_steps;
		};

		if (seeked_step < current_step) {
			set_ith(previous_snapshot_index);
		}

		/* at this point the seeked_step is either equal or greater than the current */

		const auto distance_from_previous_snapshot = seeked_step - previous_snapshot_index * snapshot_frequency_in_steps;
		const auto distance_from_current = seeked_step - current_step;

		if (distance_from_previous_snapshot < distance_from_current) {
			set_ith(previous_snapshot_index);
		}

		while (current_step < seeked_step) {
			advance_single_step(input);
		}
	}

	template <class A, class B>
	template <class MakeSnapshot>
	void snapshotted_player<A, B>::push_snapshot_if_needed(MakeSnapshot&& make_snapshot) {
		if (current_step != 0 && current_step % snapshot_frequency_in_steps == 0) {
			const auto snapshot_index = current_step / snapshot_frequency_in_steps;

			const bool valid_snapshot_exists = snapshot_index < snapshots.size();

			if (!valid_snapshot_exists) {
				snapshots.emplace_back(make_snapshot(snapshot_index));
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
	void snapshotted_player<entropy_type, B>::advance(
		const I& in,
		delta frame_delta, 
		const delta& fixed_delta
	) {
		auto steps = additional_steps;

		if (!is_paused()) {
			timer.advance(frame_delta *= speed);
			steps += timer.extract_num_of_logic_steps(fixed_delta);
		}

		while (steps--) {
			advance_single_step(in);
		}

		additional_steps = 0;
	}

}
