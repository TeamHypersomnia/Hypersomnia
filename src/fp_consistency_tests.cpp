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
#include "augs/readwrite/byte_file.h"

static_assert(std::is_same_v<streflop::Simple, float>);
static_assert(std::is_same_v<real32, float>, "Make sure you actually want to change that.");
static_assert(std::numeric_limits<real32>::is_iec559);

#define USE_THREADS 0

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

// Last stress test passes: 20000000

#if PLATFORM_UNIX
#define CANONICAL_RESULT_5000 "11000000100011010001000000011111"
#define CANONICAL_RESULT_STRESS_TEST "01000000100001010101011011001110"
#elif PLATFORM_WINDOWS
#define CANONICAL_RESULT_5000 "11000000100011010001000000011111"
#define CANONICAL_RESULT_STRESS_TEST "01000000100001010101011011001110"
#endif

struct operation_meta {
	int opcode;
	real32 op1;
	real32 op2;
	real32 r;
	real32 added_term;
	real32 total;
};

bool perform_float_consistency_tests(const float_consistency_test_settings& settings) {
	const auto passes = settings.passes;

	std::vector<operation_meta> all_ops;

	LOG("Performing %x floating point consistency passes.", passes);

	if (passes == 0) {
		return true;
	}

	auto target_report_path = augs::path_type();

	if (!settings.report_filename.empty()) {
		target_report_path = settings.report_filename;
	}

	try {
		augs::load_from_bytes(all_ops, target_report_path);
	}
	catch (const augs::file_open_error&) {

	}

	const bool supervised_test = all_ops.size() > 0;
	const bool generate_op_report = !supervised_test && !target_report_path.empty();
	bool supervision_failed_already = false;

	if (supervised_test) {
		LOG("(FP consistency test) Performing a supervised test with file: %x", target_report_path);
	}

	if (generate_op_report) {
		LOG("(FP consistency test) Generating a fp consistency test report to: %x", target_report_path);
	}

	all_ops.resize(passes);

	auto timer = augs::timer();
	auto scope = augs::scope_guard(
		[&]() {
			LOG("(FP consistency test) Time taken: %x ms", timer.get<std::chrono::milliseconds>());
		}
	);

	ensure_float_flags_hold();

	/* 
		Randomization + floating point cross-platform consistency test 
	*/


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
		auto ndt_rng = randomization::from_random_device();

		real32 trash = 4938493.f;
		real32 total = 0.f;

		for (int i = 0; i < passes; ++i) {
			const auto opcode = rng.randval(0, 21);

			real32 r = 0.f;

			switch(opcode) {
				case 0:
					OP1(0.f, 10.f);
					OP2(-10.f, 10.f);
					r = op1 - op2; 
					break;
				case 1:
					OP1(-10.f, 10.f);
					OP2(-10.f, 10.f);
					r = op1 + op2;
					break;
				case 2:
					OP1(0.f, 10.f);
					OP2(0.5f, 100.f);
					r = op1 / op2;
					break;
				case 3:
					OP1(-10.f, 10.f);
					OP2(-10.f, 10.f);
					r =  op1 * op2;
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
					OP1(-1000.f, 1000.f);
					OP2(-1000.f, 1000.f);
					r = repro::atan2(op1, op2);
					break;
				case 9:
					r = repro::acos(OP1(0.f, 1.f));
					break;
				case 10:
					OP1(-1000.f, 1000.f);
					OP2(-1000.f, 1000.f);
					r = repro::fmod(op1, op2);
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
					OP1(-1.f, 1.f);
					OP2(-1.f, 1.f);
					r = repro::copysignf(op1, op2);
					break;
				case 19:
					OP1(1.f, 8.f);
					OP2(-2.f, 2.f);
					r = repro::pow(op1, op2);
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

			auto added_term = rng.randval(-1.f, 1.f);

			if (repro::isfinite(r)) {
				added_term *= r;
			}
			else { 
				++nans;
			}


			auto division = rng.randval(0.01f, 10.f);
			added_term /= division;

			auto subtraction = rng.randval(-1.f, 1.f);
			added_term -= subtraction;

			//LOG_NVPS(total, added_term);
			if (rng.randval(0, 1)) {
				total += added_term;
			}
			else {
				total -= added_term;
			}

			if (repro::fabs(total) > 10) {
				const auto divisor = repro::pow(10.f, int(repro::log10(repro::fabs(total))));

				total /= divisor;
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

			if (supervised_test && !supervision_failed_already) {
				const auto& expected_meta = all_ops[i];
				const auto& em = expected_meta;

				std::string differences_content;

				auto to_bit_string = [](const auto& val) {
					uint32_t bits = 0;

					static_assert(sizeof(val) == sizeof(bits));
					std::memcpy(std::addressof(bits), std::addressof(val), sizeof(bits));

					return std::bitset<32>(bits).to_string();
				};

				auto verify = [&](const auto& expected, const auto& actual, const auto label) {
					const auto es = to_bit_string(expected);
					const auto as = to_bit_string(actual);

					if (es == as) {

					}
					else {
						supervision_failed_already = true;
						differences_content += typesafe_sprintf("Different %x!\nExpected: %x (%x)\nActual:   %x (%x)\n", label, expected, es, actual, as);
					}
				};

				verify(em.opcode, opcode, "opcode");
				verify(em.op1, op1, "op1");
				verify(em.op2, op2, "op2");
				verify(em.r, r, "r");
				verify(em.added_term, added_term, "added_term");
				verify(em.total, total, "total");

				if (supervision_failed_already) {
					std::string expected_vals;

					auto print_val = [&](const auto& v, const auto label) {
						expected_vals += typesafe_sprintf("%x: %x", label, v);

						if constexpr(std::is_floating_point_v<remove_cref<decltype(v)>>) {
							expected_vals += typesafe_sprintf("\t\t(%x)", to_bit_string(v));
						}

						expected_vals += "\n";
					};

					print_val(em.opcode, "opcode");
					print_val(em.op1, "op1");
					print_val(em.op2, "op2");
					print_val(em.r, "r");
					print_val(em.added_term, "added_term");
					print_val(em.total, "total");

					LOG("[%x] Failed. Expected results:\n\n%x\nDifferences:\n%x", i, expected_vals, differences_content);
				}

			}
			else if (generate_op_report) {
				auto& meta = all_ops[i];
				meta.opcode = opcode;
				meta.op1 = op1;
				meta.op2 = op2;
				meta.r = r;
				meta.added_term = added_term;
				meta.total = total;
			}
		}

		(void)trash;
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

		if (generate_op_report) {
			augs::save_as_bytes(all_ops, target_report_path);
		}
	};

#if USE_THREADS
	static const auto num_threads = 2;

	for (std::size_t i = 0; i < num_threads; ++i) {
		workers.emplace_back(work_lambda);
	}

	for (auto& w : workers) {
		w.join();
	}
#else
	work_lambda();
#endif

	if (all_succeeded) {
		LOG("(FP consistency test) Passed the test. Canonical result matches the actual results.");
	}
	else {
		LOG("(FP consistency test) Failed the test. Canonical result does not match the actual results.");
	}
	LOG_NVPS(nans);

	return all_succeeded;
}
