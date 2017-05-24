#include "tweaker.h"

#include "augs/log.h"
#include "augs/templates/container_templates.h"
#include "augs/misc/timer.h"

float TWEAK0 = 0.f;
float TWEAK1 = 0.f;
float TWEAK2 = 0.f;
float TWEAK3 = 0.f;

int CURRENT_TWEAK = 0;

bool TWEAKER_CONTROL_ENABLED = true;

void control_tweaker(augs::machine_entropy::local_type& in) {
	static augs::timer tm;
	static augs::window::event::state st;

	const auto vdt = static_cast<float>(tm.extract<std::chrono::milliseconds>());

	using namespace augs::window::event::keys;

	// auto get_tweak = [&]() {
	// 	if (CURRENT_TWEAK == 0) return &TWEAK0;
	// 	if (CURRENT_TWEAK == 1) return &TWEAK1;
	// 	if (CURRENT_TWEAK == 2) return &TWEAK2;
	// 	if (CURRENT_TWEAK == 3) return &TWEAK3;
	// };

	const auto magnify_key = key::RCTRL;
	const auto enable_key = key::RSHIFT;

	erase_if(in, [&](const augs::window::event::change ch) {
		st.apply(ch);

		bool fetched = false;

		if (ch.was_any_key_pressed()) {
			if (ch.key == enable_key) {
				TWEAKER_CONTROL_ENABLED = !TWEAKER_CONTROL_ENABLED;
				fetched = true;
			}

			if (TWEAKER_CONTROL_ENABLED) {
				// if (ch.key == key::_1) {
				// 	CURRENT_TWEAK = 0; fetched = true;
				// }
				// if (ch.key == key::_2) {
				// 	CURRENT_TWEAK = 1; fetched = true;
				// }
				// if (ch.key == key::_3) {
				// 	CURRENT_TWEAK = 2; fetched = true;
				// }
				// if (ch.key == key::_4) {
				// 	CURRENT_TWEAK = 3; fetched = true;
				// }

				if (ch.key == key::LEFT) {
					fetched = true;
				}
				if (ch.key == key::RIGHT) {
					fetched = true;
				}
				if (ch.key == key::UP) {
					fetched = true;
				}
				if (ch.key == key::DOWN) {
					fetched = true;
				}
				if (ch.key == magnify_key) {
					fetched = true;
				}
			}
		}

		return fetched;
	});

	if (TWEAKER_CONTROL_ENABLED) {
		if (st.is_set(key::LEFT)) {
			if (st.is_set(magnify_key)) {
				TWEAK0 -= vdt;
			}
			else {
				TWEAK0 -= vdt / 10;
			}
		}

		if (st.is_set(key::RIGHT)) {
			if (st.is_set(magnify_key)) {
				TWEAK0 += vdt;
			}
			else {
				TWEAK0 += vdt / 10;
			}
		}

		if (st.is_set(key::UP)) {
			if (st.is_set(magnify_key)) {
				TWEAK1 -= vdt;
			}
			else {
				TWEAK1 -= vdt / 10;
			}
		}

		if (st.is_set(key::DOWN)) {
			if (st.is_set(magnify_key)) {
				TWEAK1 += vdt;
			}
			else {
				TWEAK1 += vdt / 10;
			}
		}
	}
}

std::wstring write_tweaker_report() {
	if (TWEAKER_CONTROL_ENABLED) {
		return typesafe_sprintf(L"Tweaker mode enabled\nTWEAK0: %x\nTWEAK1: %x\nTWEAK2: %x\nTWEAK3: %x\n", TWEAK0, TWEAK1, TWEAK2, TWEAK3);
	}

	return{};
}