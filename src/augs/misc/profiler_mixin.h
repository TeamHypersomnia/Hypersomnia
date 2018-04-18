#pragma once
#include "augs/templates/introspect_declaration.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/misc/measurements.h"
#include "augs/misc/scope_guard.h"

namespace augs {
	template <class derived>
	class profiler_mixin {
	public:
		void setup_names_of_measurements() {
			auto& self = *static_cast<derived*>(this);
	
			introspect(
				[](const auto& label, auto& m){
					m.title = format_field_name(label);
				}, 
				self
			);
		}
	
		auto summary() const {
			std::vector<const time_measurements*> all_with_time;
			std::string times_summary;
			std::string amounts_summary;
			
			auto& self = *static_cast<const derived*>(this);
	
			introspect(
				[&](auto, auto& m){
					using T = std::decay_t<decltype(m)>;
					
					if constexpr(std::is_same_v<T, time_measurements>) {
						all_with_time.push_back(&m);
					}
					else {
						amounts_summary += m.summary();
					}
				}, 
				self
			);
	
			sort_range(
				all_with_time, 
				[](const auto* a, const auto* b) {
					return *a > *b; 
				}
			);
	
			for (const auto& t : all_with_time) {
				times_summary += t->summary();
			}
	
			return times_summary + amounts_summary;
		}
	};
}

inline auto measure_scope(augs::time_measurements& m) {
	m.start();
	return augs::make_scope_guard([&m]() { m.stop(); });
}