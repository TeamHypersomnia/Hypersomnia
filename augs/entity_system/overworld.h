#pragma once
#include "misc/fixed_delta_timer.h"
#include <vector>
#include <random>

namespace augs {
	class renderer;

	class overworld {
		timer frame_timer;
		timer frame_timestamper;
		double delta_ms = 0.0;
		double last_frame_timestamp_seconds = 0.0;

		std::mt19937 deterministic_generator;
		std::mt19937 drawing_time_generator;
		std::mt19937* current_generator = nullptr;
	public:
		std::vector<std::pair<unsigned long long, double>> timestep_alterations_at_steps;

		fixed_delta_timer delta_timer;

		overworld();

		void enable_drawing_time_random_generator();
		void enable_deterministic_random_generator();
		std::mt19937& get_current_generator();

		double delta_milliseconds() const;
		double fixed_delta_milliseconds() const;
		double delta_seconds() const;
		double view_interpolation_ratio() const;
		double frame_timestamp_seconds() const;

		void assign_frame_time_to_delta_for_drawing_time_systems();
		void restore_fixed_delta();

		void configure_stepping(float fps, int max_updates_per_step);
	};
}