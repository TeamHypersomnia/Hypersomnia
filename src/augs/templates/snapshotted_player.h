#pragma once
#include <map>
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/timing/fixed_delta_timer.h"
#include "augs/misc/timing/delta.h"
#include "augs/templates/snapshotted_player_step_type.h"

namespace augs {
	struct introspection_access;

	template <class entropy_type, class Step, class MakeSnapshot>
	struct snapshotted_advance_input {
		entropy_type& total_collected;
		Step step;
		MakeSnapshot make_snapshot;

		snapshotted_advance_input(
			entropy_type& total_collected,
			Step&& step,
			MakeSnapshot&& make_snapshot
		) :
			total_collected(total_collected),
			step(std::move(step)),
			make_snapshot(std::move(make_snapshot))
		{}
	};

	template <
		class entropy_type,
		class snapshot_type
	>
	class snapshotted_player {
	public: 
		using step_type = snapshotted_player_step_type;

	protected:
		enum class advance_type {
			PAUSED,

			RECORDING,
			REPLAYING
		};

		friend introspection_access;

		// GEN INTROSPECTOR class snapshotted_player class A class B
		std::map<step_type, entropy_type> step_to_entropy;
		advance_type advance_mode = advance_type::PAUSED;
		step_type current_step = 0;

		unsigned snapshot_frequency_in_steps = 3000;
		std::vector<snapshot_type> snapshots;

		fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

		double speed = 1.0;
		int additional_steps = 0;
		// END GEN INTROSPECTOR

		template <class MakeSnapshot>
		void push_snapshot_if_needed(MakeSnapshot&&);

		template <class I>
		void advance_single_step(const I& input);

	protected:
		/* Returns the number of steps performed */

		template <class I>
		int advance(
			const I& input,
			delta frame_delta, 
			const delta& fixed_delta
		);

	public:
		auto get_current_step() const;
		auto get_total_steps() const;

		template <
			class I,
			class SetSnapshot
		>
		void seek_to(
			step_type, 
			const I& input,
			SetSnapshot&& set_snapshot
		);

		void seek_to(step_type);

		const fixed_delta_timer& get_timer() const;

		void pause();
		void resume();

		void finish();

		bool is_paused() const;

		void request_steps(int amount);

		void begin_replaying();
		void begin_recording();

		double get_speed() const;
		void set_speed(double);
	};
}

