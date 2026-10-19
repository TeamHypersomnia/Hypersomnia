#pragma once
/* BENCH TEMP - CPU timers of the bots' logic per step */
#include <chrono>
#include <cstdint>
#include <algorithm>
#include "augs/log.h"

namespace bench_cpu {
	enum slot {
		STEP,
		EXEC_CMDS,
		AI_TEAM,
		AI_BOT,
		AI_POST,
		PATH_DIRECT,
		PATH_MANY,
		PATH_MANY_FULL,
		RANDOM_WP,
		COUNT
	};

	inline const char* names[COUNT] = {
		"whole step (standard_solver)",
		"execute_player_commands (all AI)",
		"update_arena_mode_ai_team",
		"update_arena_mode_ai (per bot)",
		"post_solve_arena_mode_ai",
		"find_path_across_islands_direct",
		"find_path_across_islands_many",
		"find_path_across_islands_many_full",
		"generate_random_waypoints"
	};

	struct acc_t {
		double ns = 0;
		long calls = 0;
		double max_call_ns = 0;
		double step_ns = 0;
		double max_step_ns = 0;
	};

	inline acc_t acc[COUNT];
	inline long steps = 0;
	inline auto window_start = std::chrono::steady_clock::now();
	inline auto run_start = std::chrono::steady_clock::now();

	inline bool report_and_reset() {
		const auto now = std::chrono::steady_clock::now();

		for (auto& a : acc) {
			a.max_step_ns = std::max(a.max_step_ns, a.step_ns);
			a.step_ns = 0;
		}

		if (std::chrono::duration<double>(now - window_start).count() < 10.0) {
			++steps;
			return false;
		}

		LOG("BENCHCPU window: %x steps (since start %2f s)", steps, std::chrono::duration<double>(now - run_start).count());

		for (int i = 0; i < COUNT; ++i) {
			const auto& a = acc[i];

			if (a.calls == 0) {
				continue;
			}

			LOG(
				"BENCHCPU %x: %4f ms/step, %2f calls/step, max call %4f ms, max per step %4f ms",
				names[i],
				a.ns / steps / 1e6,
				static_cast<double>(a.calls) / steps,
				a.max_call_ns / 1e6,
				a.max_step_ns / 1e6
			);
		}

		for (auto& a : acc) {
			a = acc_t();
		}

		steps = 1;
		window_start = now;
		return true;
	}

	struct scope {
		slot s;
		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();

		scope(const slot s) : s(s) {}

		~scope() {
			const auto ns = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t).count());
			auto& a = acc[s];
			a.ns += ns;
			a.calls += 1;
			a.max_call_ns = std::max(a.max_call_ns, ns);
			a.step_ns += ns;
		}
	};
}
