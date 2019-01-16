#include "augs/math/repro_math.h"

#include "augs/log.h"
#include "augs/ensure.h"

#if !defined(STREFLOP_SSE)
#error "STREFLOP_SSE was not defined."
#endif

#include "fp_consistency_tests.h"
#include "augs/misc/randomization.h"
#include "augs/misc/timing/timer.h"
#include "augs/misc/scope_guard.h"

static_assert(std::is_same_v<repro::Simple, float>);

void setup_float_flags() {
	repro::fesetround(repro::FE_TONEAREST);
	repro::streflop_init<repro::Simple>();
}

void ensure_float_flags_hold() {
	ensure_eq(repro::fegetround(), repro::FE_TONEAREST);
}

#include <thread>
#include <bitset>
#include <random>

#define CANONICAL_RESULT "11000110001101111100111000111011"

bool perform_float_consistency_tests() {
	auto timer = augs::timer();
	auto scope = augs::scope_guard(
		[&]() {
			LOG("FP consistency test took: %x ms", timer.get<std::chrono::milliseconds>());
		}
	);

	ensure_float_flags_hold();

	/* 
		Randomization + floating point cross-platform consistency test 
	*/


	static constexpr auto num_operations = 5000;
	static const auto num_threads = 2 * std::thread::hardware_concurrency();
	(void)num_threads;

	/* Perform a random walk with randomized mathematical operations */

	std::vector<std::thread> workers;

	std::atomic<bool> all_succeeded = true;

	auto work_lambda = [&]() {
		auto rng = randomization(1337u);
		auto ndt_rng = randomization(std::random_device()());

		repro::Simple trash = 4938493.f;
		repro::Simple total = 0.f;

		for (int i = 0; i < num_operations; ++i) {
			const auto opcode = rng.randval(0, 19);

			repro::Simple r = 0.f;

			switch(opcode) {
				case 0:
					r = rng.randval(0.f, 10.f) - rng.randval(-10.f, 10.f);
					break;
				case 1:
					r = rng.randval(-10.f, 10.f) + rng.randval(-10.f, 10.f);
					break;
				case 2:
					r = rng.randval(0.f, 10.f) / rng.randval(0.5f, 100.f);
					break;
				case 3:
					r = rng.randval(-10.f, 10.f) * rng.randval(-10.f, 10.f);
					break;
				case 4:
					r = rng.randval(0.f, 10.f) / 0.f;
					break;
				case 5:
					r = repro::sqrt(rng.randval(-30.f, -54.f));
					break;
				case 6:
					r = repro::sin(rng.randval(-1000.f, 1000.f));
					break;
				case 7:
					r = repro::cos(rng.randval(-1000.f, 1000.f));
					break;
				case 8:
					r = repro::atan2(rng.randval(-1000.f, 1000.f), rng.randval(-1000.f, 1000.f));
					break;
				case 9:
					r = repro::acos(rng.randval(0.f, 1.f));
					break;
				case 10:
					r = repro::fmod(rng.randval(-1000.f, 1000.f), rng.randval(-1000.f, 1000.f));
					break;
				case 11:
					r = repro::nearbyint(rng.randval(-1000.f, 1000.f));
					break;
				case 12:
					r = repro::exp(rng.randval(-10.f, 4.f));
					break;
				case 13:
					r = repro::exp2(rng.randval(-10.f, 4.f));
					break;
				case 14:
					r = repro::log(rng.randval(-10.f, 100000.f));
					break;
				case 15:
					r = repro::log2(rng.randval(-10.f, 100000.f));
					break;
				case 16:
					r = repro::trunc(rng.randval(-1000.f, 1000.f));
					break;
				case 17:
					r = repro::fabs(rng.randval(-1000.f, 1000.f));
					break;
				case 18:
					r = repro::copysignf(rng.randval(-1.f, 1.f), rng.randval(-1.f, 1.f));
					break;
				case 19:
					r = repro::pow(rng.randval(1.f, 8.f), rng.randval(-2.f, 2.f));
					break;

				default:
					LOG("WRONG OPCODE: %x!!!", opcode);
					all_succeeded.store(false);
					return;
			};

			auto addition = rng.randval(-1.f, 1.f);

			if (repro::isfinite(r)) {
				addition *= r;
			}
			else {

			}

			//LOG_NVPS(total, addition);
			total += addition;

			{
				trash = 
					repro::sin(ndt_rng.randval(-1000.f, 1000.f))
					* repro::cos(ndt_rng.randval(-1000.f, 1000.f))
					* repro::acos(ndt_rng.randval(0.f, 1.f))
					* repro::pow(ndt_rng.randval(1.f, 8.f), ndt_rng.randval(-2.f, 2.f))
					* repro::log(ndt_rng.randval(1.f, 1000.f))
					/ ndt_rng.randval(-0.001f, 0.001f)
				;
			}
		}

		LOG("Trash stuff: %x", trash);

		uint32_t bits = 0;
		std::memcpy(std::addressof(bits), std::addressof(total), sizeof(bits));

		const auto bits_representation = std::bitset<32>(bits);
		const auto test_result_representation = bits_representation.to_string();

		if (test_result_representation != CANONICAL_RESULT) {
			LOG(
				"(FP consistency test) Representations differ!\nCanonical: %x\nActual: %x (%x)",
				CANONICAL_RESULT,
				test_result_representation,
				total
			);

			all_succeeded.store(false);
		}
	};

	for (std::size_t i = 0; i < num_threads; ++i) {
		workers.emplace_back(work_lambda);
	}

	for (auto& w : workers) {
		w.join();
	}
	
	if (all_succeeded) {
		LOG("(FP consistency test) Passed the test. Canonical results match the actual results.");
	}

	return true;
}
