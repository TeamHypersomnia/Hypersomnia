#pragma once
#include <map>
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/timing/fixed_delta_timer.h"
#include "augs/misc/timing/delta.h"
#include "augs/templates/snapshotted_player_step_type.h"
#include "augs/templates/snapshotted_player_settings.h"

namespace augs {
	struct introspection_access;

	template <class Step, class RecordEntropy, class MakeSnapshot>
	struct snapshotted_advance_input {
		Step step;
		RecordEntropy record_entropy;
		MakeSnapshot make_snapshot;
		const snapshotted_player_settings& settings;

		snapshotted_advance_input(
			Step&& step,
			RecordEntropy&& record_entropy,
			MakeSnapshot&& make_snapshot,
			const snapshotted_player_settings& settings
		) :
			step(std::move(step)),
			record_entropy(std::move(record_entropy)),
			make_snapshot(std::move(make_snapshot)),
			settings(settings)
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
		using snapshots_type = std::map<step_type, snapshot_type>;

		// GEN INTROSPECTOR class augs::snapshotted_player class A class B
		std::map<step_type, entropy_type> step_to_entropy;

	private:
		advance_type advance_mode = advance_type::PAUSED;
		step_type current_step = 0;
		snapshots_type snapshots;
		fixed_delta_timer timer = { 30, augs::lag_spike_handling_type::CATCH_UP };

		double speed = 1.0;
		step_type additional_steps = 0;
		// END GEN INTROSPECTOR

		template <class MakeSnapshot>
		void push_snapshot_if_needed(MakeSnapshot&&, unsigned interval_in_steps);

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

		void clear_later_entropies();

		template <class F>
		void for_each_later_entropy(F&&);

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
		const snapshots_type& get_snapshots() const;

		void pause();
		void resume();

		void finish();

		bool is_paused() const;
		bool is_recording() const;
		bool is_replaying() const;

		void request_steps(step_type amount);

		void begin_replaying();
		void begin_recording();

		double get_speed() const;
		void set_speed(double);
	};
}

