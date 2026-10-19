#pragma once
/* BENCH TEMP - per frame CPU time of the thread pool jobs */
#include <atomic>
#include <chrono>
#include "augs/log.h"

namespace bench_jobs {
	enum slot { FOW, LIGHT, ILLUM, GAME_GUI, POST_GUI, PERF_DETAILS, HELP_WALL, VIS_RAYCAST_LOOP, VIS_CALLS, VIS_VERTS, VIS_RAYS, COUNT };
	inline const char* names[COUNT] = { "fow_job", "light_jobs (sum)", "illuminated_rendering_job", "game_gui_job", "post_game_gui_job", "show_performance_details_job", "help+wait wall", "calc_visibility ray_cast loop", "calc_visibility calls (x1e6)", "vertices (x1e6)", "rays (x1e6)" };
	inline std::atomic<long long> ns[COUNT];
	inline std::atomic<long long> max_ns[COUNT];
	inline long frames = 0;
	inline auto last = std::chrono::steady_clock::now();
	inline auto start = std::chrono::steady_clock::now();

	struct scope {
		slot s;
		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
		scope(const slot s) : s(s) {}
		~scope() {
			const auto d = static_cast<long long>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t).count());
			ns[s] += d;
			auto m = max_ns[s].load();
			while (d > m && !max_ns[s].compare_exchange_weak(m, d)) {}
		}
	};

	inline void end_frame() {
		++frames;
		const auto now = std::chrono::steady_clock::now();

		if (std::chrono::duration<double>(now - last).count() > 10.0) {
			last = now;

			if (std::chrono::duration<double>(now - start).count() > 12.0) {
				LOG("BENCHJOBS frames=%x", frames);

				for (int i = 0; i < COUNT; ++i) {
					LOG("BENCHJOBS %x: %4f ms/frame, max single %4f ms", names[i], ns[i].load() / 1e6 / frames, max_ns[i].load() / 1e6);
				}
			}

			for (int i = 0; i < COUNT; ++i) {
				ns[i] = 0;
				max_ns[i] = 0;
			}

			frames = 0;
		}
	}
}
