#pragma once
/* BENCH TEMP - per pass GPU timing: a GL_TIMESTAMP at every change of FBO, program or light_pass */
#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include "augs/log.h"
#include "augs/graphics/OpenGL_includes.h"

namespace bench_gpu {
	struct key_t {
		int fbo = 0;
		int prog = 0;
		int tag = -1;
		bool operator<(const key_t& b) const { return std::tie(fbo, prog, tag) < std::tie(b.fbo, b.prog, b.tag); }
	};

	struct stat_t {
		double ns = 0;
		double tris = 0;
		double draws = 0;
	};

	inline bool enabled = std::getenv("BENCH_GPU") == nullptr || std::string(std::getenv("BENCH_GPU")) != "0";
	inline std::vector<double> frame_times;
	inline auto ft_last = std::chrono::steady_clock::now();
	inline key_t cur;
	inline std::vector<GLuint> pool;
	inline std::vector<key_t> keys;
	inline std::vector<double> seg_tris;
	inline std::vector<double> seg_draws;
	inline std::size_t used = 0;
	inline std::map<key_t, stat_t> acc;
	inline double total_ns = 0;
	inline long frames = 0;
	inline std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	inline std::chrono::steady_clock::time_point last_report = start;
	inline double frame_s_sum = 0;

	inline void mark() {
		if (!enabled) {
			return;
		}

		if (used >= pool.size()) {
			const auto old = pool.size();
			pool.resize(old + 256);
			glGenQueries(256, pool.data() + old);
		}

		glQueryCounter(pool[used], GL_TIMESTAMP);
		keys.resize(used + 1);
		seg_tris.resize(used + 1);
		seg_draws.resize(used + 1);
		keys[used] = cur;
		seg_tris[used] = 0;
		seg_draws[used] = 0;
		++used;
	}

	inline void on_draw(const double tris) {
		if (used > 0) {
			seg_tris[used - 1] += tris;
			seg_draws[used - 1] += 1;
		}
	}

	inline void set_fbo(const int id) { cur.fbo = id; mark(); }
	inline void set_prog(const int id) { cur.prog = id; cur.tag = -1; mark(); }
	inline void set_tag(const int t) { if (t != cur.tag) { cur.tag = t; mark(); } }

	inline void begin_frame() {
		used = 0;
		mark();
	}

	inline void frame_stats(const double frame_s) {
		const auto now = std::chrono::steady_clock::now();
		const auto since_start = std::chrono::duration<double>(now - start).count();

		if (since_start < 8.0) {
			ft_last = now;
			return;
		}

		frame_times.push_back(frame_s * 1000.0);

		if (std::chrono::duration<double>(now - ft_last).count() > 5.0) {
			ft_last = now;
			auto v = frame_times;
			std::sort(v.begin(), v.end());
			double sum = 0;
			long over8 = 0;
			for (const auto x : v) { sum += x; if (x > 8.0) { ++over8; } }
			auto pct = [&](const double p) { return v[std::min(v.size() - 1, static_cast<std::size_t>(p * v.size()))]; };
			LOG("BENCHFRAME t=%2f n=%x fps %2f | p50 %4f p90 %4f p99 %4f max %4f ms | >8ms %x", since_start, v.size(), 1000.0 * v.size() / sum, pct(0.5), pct(0.9), pct(0.99), v.back(), over8);
			frame_times.clear();
		}
	}

	inline void end_frame(const double frame_s) {
		frame_stats(frame_s);

		if (!enabled || used == 0) {
			return;
		}

		mark();

		std::vector<GLuint64> ts(used);

		for (std::size_t i = 0; i < used; ++i) {
			glGetQueryObjectui64v(pool[i], GL_QUERY_RESULT, &ts[i]);
		}

		const auto now = std::chrono::steady_clock::now();
		const auto since_start = std::chrono::duration<double>(now - start).count();

		if (since_start < 8.0) {
			last_report = now;
			return;
		}

		for (std::size_t i = 0; i + 1 < used; ++i) {
			auto& s = acc[keys[i]];
			s.ns += static_cast<double>(ts[i + 1] - ts[i]);
			s.tris += seg_tris[i];
			s.draws += seg_draws[i];
		}

		total_ns += static_cast<double>(ts[used - 1] - ts[0]);
		frame_s_sum += frame_s;
		++frames;

		if (std::chrono::duration<double>(now - last_report).count() > 10.0) {
			last_report = now;

			std::vector<std::pair<key_t, stat_t>> sorted(acc.begin(), acc.end());
			std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.second.ns > b.second.ns; });

			LOG("BENCHSEG frames=%x total_gpu %4f ms fps(with readback) %2f", frames, total_ns / frames / 1e6, frames / frame_s_sum);

			for (std::size_t i = 0; i < std::min<std::size_t>(sorted.size(), 25); ++i) {
				const auto& k = sorted[i].first;
				const auto& s = sorted[i].second;
				LOG("BENCHSEG fbo=%x prog=%x tag=%x : %4f ms tris %x draws %x", k.fbo, k.prog, k.tag, s.ns / frames / 1e6, static_cast<long>(s.tris / frames), static_cast<long>(s.draws / frames));
			}
		}
	}
}
