#pragma once
#include "augs/templates/introspect_declaration.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/measurements.h"
#include "augs/misc/scope_guard.h"

namespace augs {
	template <class derived>
	class profiler_mixin {
	public:
		void setup_names_of_measurements() {
			auto& self = *static_cast<derived*>(this);
	
			introspect(
				[](const auto label, auto& m){
					m.title = to_wstring(format_field_name(std::string(label)));
				}, 
				self
			);
		}
	
		auto summary() const {
			std::vector<std::reference_wrapper<const time_measurements>> all_with_time;
			std::wstring times_summary;
			std::wstring amounts_summary;
			
			auto& self = *static_cast<const derived*>(this);
	
			introspect(
				[&](auto, auto& m){
					using T = std::decay_t<decltype(m)>;
					
					if constexpr(std::is_same_v<T, time_measurements>) {
						all_with_time.push_back(std::cref(m));
					}
					else {
						amounts_summary += m.summary();
					}
				}, 
				self
			);
	
			sort_range(
				all_with_time, 
				[](const auto& a, const auto& b) { 
					return a.get() > b.get(); 
				}
			);
	
			for (const auto& t : all_with_time) {
				times_summary += t.get().summary();
			}
	
			return times_summary + amounts_summary;
		}
	};
}

inline auto measure_scope(augs::time_measurements& m) {
	m.start();
	return augs::make_scope_guard([&m]() { m.stop(); });
}