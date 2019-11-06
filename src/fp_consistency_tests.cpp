#define FORCE_DISABLE_STREFLOP 1
#include "augs/math/repro_math.h"

#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/ensure_rel.h"

#if !defined(STREFLOP_SSE)
#error "STREFLOP_SSE was not defined."
#endif

#include "fp_consistency_tests.h"
#include "augs/misc/randomization.h"
#include "augs/misc/timing/timer.h"
#include "augs/misc/scope_guard.h"

static_assert(std::is_same_v<streflop::Simple, float>);

#if !USE_STREFLOP
#include <cfenv>
#endif

void setup_float_flags() {
#if USE_STREFLOP
	streflop::fesetround(streflop::FE_TONEAREST);
	streflop::streflop_init<streflop::Simple>();
#else
	std::fesetround(FE_TONEAREST);
#endif
}

void ensure_float_flags_hold() {
#if USE_STREFLOP
	ensure_eq(streflop::fegetround(), static_cast<int>(streflop::FE_TONEAREST));
#else
	ensure_eq(std::fegetround(), FE_TONEAREST);
#endif
}

#include <thread>
#include <atomic>
#include <bitset>
#include <random>

// Last stress test passes: 50000000

#if PLATFORM_UNIX
#define CANONICAL_RESULT_5000 "00111111110010000001110101110001"
#define CANONICAL_RESULT_STRESS_TEST "00111110100010111001110011111010"
#elif PLATFORM_WINDOWS
#define CANONICAL_RESULT_5000 "00111111110010000001110101110001"
#define CANONICAL_RESULT_STRESS_TEST "00111110100010111001110011111010"
#endif

bool perform_float_consistency_tests(const int passes) {
	if (passes == 0) {
		return true;
	}

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


	static const auto num_threads = 2;

	/* Perform a random walk with randomized mathematical operations */

	std::vector<std::thread> workers;

	std::atomic<bool> all_succeeded = true;

	int nans = 0;

	float op1 = 0.f;
	float op2 = 0.f;

#define OP1(a, b) (op1 = rng.randval(a+repro::fabs(total), b+repro::fabs(total)))
#define OP2(a, b) (op2 = rng.randval(a+repro::fabs(total), b+repro::fabs(total)))

	auto work_lambda = [&]() {
		auto rng = randomization(1337u);
		auto ndt_rng = randomization(std::random_device()());

		real32 trash = 4938493.f;
		real32 total = 0.f;

		for (int i = 0; i < passes; ++i) {
			const auto opcode = rng.randval(0, 21);

			real32 r = 0.f;

			switch(opcode) {
				case 0:
					r = OP1(0.f, 10.f) - OP2(-10.f, 10.f);
					break;
				case 1:
					r = OP1(-10.f, 10.f) + OP2(-10.f, 10.f);
					break;
				case 2:
					r = OP1(0.f, 10.f) / OP2(0.5f, 100.f);
					break;
				case 3:
					r = OP1(-10.f, 10.f) * OP2(-10.f, 10.f);
					break;
				case 4:
					r = OP1(0.f, 10.f) / 0.f;
					break;
				case 5:
					r = repro::sqrt(OP1(0.f, 0.0001f));
					break;
				case 6:
					r = repro::sin(OP1(-0.0001f, 0.0001f));
					break;
				case 7:
					r = repro::cos(OP1(-1000.f, 1000.f));
					break;
				case 8:
					r = repro::atan2(OP1(-1000.f, 1000.f), OP2(-1000.f, 1000.f));
					break;
				case 9:
					r = repro::acos(OP1(0.f, 1.f));
					break;
				case 10:
					r = repro::fmod(OP1(-1000.f, 1000.f), OP2(-1000.f, 1000.f));
					break;
				case 11:
					r = repro::nearbyint(OP1(-1000.f, 1000.f));
					break;
				case 12:
					r = repro::exp(OP1(-10.f, 4.f));
					break;
				case 13:
					r = repro::exp2(OP1(-10.f, 4.f));
					break;
				case 14:
					r = repro::log(OP1(-10.f, 100000.f));
					break;
				case 15:
					r = repro::log2(OP1(0.001f, 0.0000001f));
					break;
				case 16:
					r = repro::trunc(OP1(-1.f, 1.f));
					break;
				case 17:
					r = repro::fabs(OP1(-1.f, 1.f));
					break;
				case 18:
					r = repro::copysignf(OP1(-1.f, 1.f), OP2(-1.f, 1.f));
					break;
				case 19:
					r = repro::pow(OP1(1.f, 8.f), OP2(-2.f, 2.f));
					break;
				case 20:
					r = repro::sqrt(OP1(-5400.f, 0.f));
					break;
				case 21:
					r = repro::sqrt(OP1(0.f, 5000.f));
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
				++nans;
			}

			auto division = rng.randval(0.01f, 10.f);
			addition /= division;

			auto subtraction = rng.randval(-1.f, 1.f);
			addition -= subtraction;

			//LOG_NVPS(total, addition);
			total += addition;

			if (repro::fabs(total) > 10) {
				total /= repro::pow(10.f, int(repro::log10(repro::fabs(total))));
			}

			if (!repro::isfinite(r)) {
				total /= 1000;
			}

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

#if 0
		LOG("Trash stuff: %x", trash);
#endif

		uint32_t bits = 0;
		std::memcpy(std::addressof(bits), std::addressof(total), sizeof(bits));

		const auto bits_representation = std::bitset<32>(bits);
		const auto test_result_representation = bits_representation.to_string();

		const auto canonical_v = passes == 5000 ? CANONICAL_RESULT_5000 : CANONICAL_RESULT_STRESS_TEST;

		if (test_result_representation != canonical_v) {
			LOG(
				"(FP consistency test) Representations differ!\nCanonical: %x\nActual: %x (%x)",
				canonical_v,
				test_result_representation,
				total
			);

			all_succeeded.store(false);
		}
		else {
			LOG("(FP consistency test) Representations match. Achieved value: %x", total);
		}
	};

	for (std::size_t i = 0; i < num_threads; ++i) {
		workers.emplace_back(work_lambda);
	}

	for (auto& w : workers) {
		w.join();
	}
	
	if (all_succeeded) {
		LOG("(FP consistency test) Passed the test. Canonical result matches the actual results.");
	}
	else {
		LOG("(FP consistency test) Failed the test. Canonical result does not match the actual results.");
	}
	LOG_NVPS(nans);

	return all_succeeded;
}
