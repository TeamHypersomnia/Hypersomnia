/*
	BENCH TEMP - paste into src/work.cpp, replacing the loop:

		rendering_result.clear();

		for (auto& r : read_buffer.renderers.all) {
			renderer_backend.perform(...);
		}

	Replace bench_skip_THING with the toggle flag of the measured code.
	Needs #include "augs/graphics/OpenGL_includes.h" in work.cpp.
*/

				rendering_result.clear();

				/* BENCH TEMP */
				struct bench_state_t {
					bool inited = false;
					GLuint queries[8] = {};
					int query_variant[8] = {};
					bool query_pending[8] = {};
					int slot = 0;
					std::chrono::steady_clock::time_point start;
					std::chrono::steady_clock::time_point last_frame;
					int last_phase = -1;
					double gpu_ns_sum[2] = {};
					long gpu_n[2] = {};
					double cpu_s_sum[2] = {};
					long cpu_n[2] = {};
				};

				static bench_state_t bench;

				const auto bench_now = std::chrono::steady_clock::now();

				if (!bench.inited) {
					glGenQueries(8, bench.queries);
					bench.inited = true;
					bench.start = bench_now;
					bench.last_frame = bench_now;
				}

				const double bench_warmup_s = 8.0;
				const double bench_phase_s = 3.0;
				const double bench_discard_s = 0.5;

				const double bench_elapsed = std::chrono::duration<double>(bench_now - bench.start).count() - bench_warmup_s;
				const double bench_frame_s = std::chrono::duration<double>(bench_now - bench.last_frame).count();
				bench.last_frame = bench_now;

				const int bench_phase = bench_elapsed < 0.0 ? -1 : static_cast<int>(bench_elapsed / bench_phase_s);
				const int bench_variant = bench_phase < 0 ? 0 : bench_phase % 2;
				const bool bench_measuring = bench_phase >= 0 && (bench_elapsed - bench_phase * bench_phase_s) > bench_discard_s;

				/*
					Variant 0 = WITH the measured thing, 1 = WITHOUT.
				*/

				bench_skip_THING = bench_variant == 1;

				if (bench_phase != bench.last_phase && bench_phase > 0 && bench_phase % 2 == 0) {
					auto avg = [](const double s, const long n) { return n > 0 ? s / n : 0.0; };

					const auto with_gpu = avg(bench.gpu_ns_sum[0], bench.gpu_n[0]) / 1e6;
					const auto without_gpu = avg(bench.gpu_ns_sum[1], bench.gpu_n[1]) / 1e6;
					const auto with_cpu = avg(bench.cpu_s_sum[0], bench.cpu_n[0]) * 1e3;
					const auto without_cpu = avg(bench.cpu_s_sum[1], bench.cpu_n[1]) * 1e3;

					LOG(
						"BENCH cycles=%x WITH: gpu %4f ms fps %2f (n=%x) | WITHOUT: gpu %4f ms fps %2f (n=%x) | gpu diff %4f ms",
						bench_phase / 2,
						with_gpu, with_cpu > 0 ? 1000.0 / with_cpu : 0.0, bench.gpu_n[0],
						without_gpu, without_cpu > 0 ? 1000.0 / without_cpu : 0.0, bench.gpu_n[1],
						with_gpu - without_gpu
					);
				}

				bench.last_phase = bench_phase;

				if (bench_measuring) {
					bench.cpu_s_sum[bench_variant] += bench_frame_s;
					++bench.cpu_n[bench_variant];
				}

				{
					const auto s = bench.slot;

					if (bench.query_pending[s]) {
						GLuint64 ns = 0;
						glGetQueryObjectui64v(bench.queries[s], GL_QUERY_RESULT, &ns);

						if (bench.query_variant[s] >= 0) {
							bench.gpu_ns_sum[bench.query_variant[s]] += static_cast<double>(ns);
							++bench.gpu_n[bench.query_variant[s]];
						}

						bench.query_pending[s] = false;
					}

					glBeginQuery(GL_TIME_ELAPSED, bench.queries[s]);
				}

				for (auto& r : read_buffer.renderers.all) {
					renderer_backend.perform(
						rendering_result,
						r.commands.data(),
						r.commands.size(),
						r.dedicated
					);
				}

				{
					glEndQuery(GL_TIME_ELAPSED);
					bench.query_pending[bench.slot] = true;
					bench.query_variant[bench.slot] = bench_measuring ? bench_variant : -1;
					bench.slot = (bench.slot + 1) % 8;
				}
