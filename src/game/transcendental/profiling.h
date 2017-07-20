#pragma once
#include "augs/misc/measurements.h"
#include "augs/misc/scope_guard.h"

template <class derived>
class profiler_mixin {
public:
	void setup_names_of_measurements() {
		auto& self = *static_cast<derived*>(this);

		augs::introspect(
			[](const auto label, auto& m){
				m.title = to_wstring(format_field_name(std::string(label)));
			}, 
			self
		);
	}

	auto summary() const {
		std::vector<std::reference_wrapper<const augs::time_measurements>> time_measurements;
		std::wstring times_summary;
		std::wstring amounts_summary;
		
		auto& self = *static_cast<const derived*>(this);

		augs::introspect(
			[&](auto, auto& m){
				using T = std::decay_t<decltype(m)>;
				
				if constexpr(std::is_same_v<T, augs::time_measurements>) {
					time_measurements.push_back(std::cref(m));
				}
				else {
					amounts_summary += m.summary();
				}
			}, 
			self
		);

		reverse_container(
			sort_container(time_measurements, [](const auto& a, const auto& b) { return a.get() < b.get(); })
		);

		for (const auto& t : time_measurements) {
			times_summary += t.get().summary();
		}

		return times_summary + amounts_summary;
	}
};

class cosmic_profiler : public profiler_mixin<cosmic_profiler> {
public:
	cosmic_profiler();

	// GEN INTROSPECTOR class cosmic_profiler
	augs::time_measurements complete_reinference = 1;

	augs::amount_measurements<std::size_t> raycasts;
	augs::amount_measurements<std::size_t> entropy_length;

	augs::time_measurements logic;
	augs::time_measurements rendering;
	augs::time_measurements camera_query;
	augs::time_measurements gui;
	augs::time_measurements interpolation;
	augs::time_measurements visibility;
	augs::time_measurements physics;
	augs::time_measurements particles;
	augs::time_measurements ai;
	augs::time_measurements pathfinding;

	augs::time_measurements total_load = 1;
	augs::time_measurements reading_savefile = 1;
	augs::time_measurements deserialization_pass = 1;

	augs::time_measurements total_save = 1;
	augs::time_measurements size_calculation_pass = 1;
	augs::time_measurements memory_allocation_pass = 1;
	augs::time_measurements serialization_pass = 1;
	augs::time_measurements writing_savefile = 1;

	augs::amount_measurements<std::size_t> delta_bytes = 1;

	augs::time_measurements duplication = 1;

	augs::time_measurements delta_encoding = 1;
	augs::time_measurements delta_decoding = 1;
	// END GEN INTROSPECTOR
};

class session_profiler : public profiler_mixin<session_profiler> {
public:
	session_profiler();

	// GEN INTROSPECTOR class session_profiler
	augs::time_measurements fps;
	augs::time_measurements frame;
	augs::time_measurements local_entropy;
	augs::amount_measurements<std::size_t> triangles;
	// END GEN INTROSPECTOR
};

class network_profiler : public profiler_mixin<network_profiler> {
public:
	// GEN INTROSPECTOR class network_profiler
	augs::time_measurements unpack_remote_steps;
	augs::time_measurements sending_commands_and_predict;
	augs::time_measurements sending_packets;
	augs::time_measurements remote_entropy;
	// END GEN INTROSPECTOR
};

auto measure_scope(augs::time_measurements& m) {
	m.start();
	return augs::make_scope_guard([&m]() { m.stop(); });
}