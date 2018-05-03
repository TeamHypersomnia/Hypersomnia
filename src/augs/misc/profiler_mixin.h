#pragma once
#include "augs/templates/introspect_declaration.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/misc/measurements.h"

namespace augs {
	template <class derived>
	class profiler_mixin {
		template <class S, class F>
		static void for_each_measurement(F&& callback, S& s) {
			augs::introspect(
				augs::recursive([&](auto self, const auto& label, auto& m) {
					using T = remove_cref<decltype(m)>;

					if constexpr(has_title_v<T>) {
						(void)self;
						callback(label, m);
					}
					else {
						augs::introspect(augs::recursive(self), m);
					}
				}),
				s
			);
		}

	public:
		void setup_names_of_measurements() {
			auto& self = *static_cast<derived*>(this);
	
			for_each_measurement(
				[](const auto& label, auto& m) {
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
	
			for_each_measurement(
				[&](auto, const auto& m) {
					using T = remove_cref<decltype(m)>;
					
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