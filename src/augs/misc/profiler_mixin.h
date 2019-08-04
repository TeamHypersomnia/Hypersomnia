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
				[&](auto& label, auto& m) {
					using T = remove_cref<decltype(m)>;

					if constexpr(has_title_v<T>) {
						callback(label, m);
					}
					else {
						for_each_measurement(std::forward<F>(callback), m);
					}
				},
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
	
		void prepare_summary_info() {
			auto& self = *static_cast<derived*>(this);

			for_each_measurement(
				[](const auto&, auto& m) {
					m.prepare_summary_info();
				},
				self
			);
		}

		void summary(std::string& output) const {
			thread_local std::vector<const time_measurements*> all_with_time;
			thread_local std::string amounts_summary;

			auto& times_summary = output;

			all_with_time.clear();
			times_summary.clear();
			amounts_summary.clear();
			
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
					return a->get_summary_info().value > b->get_summary_info().value;
				}
			);
	
			for (const auto& t : all_with_time) {
				times_summary += t->summary();
			}
	
			output += amounts_summary;
		}
	};
}