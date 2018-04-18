#pragma once
#include "augs/misc/scope_guard.h"

namespace augs {
	class field_name_tracker {
		std::vector<std::string> fields;

		void pop() {
			fields.pop_back();
		}

	public:
		auto track(const std::string& name) {
			fields.push_back(name);
			return augs::make_scope_guard([this](){ pop(); });
		}

		auto get_full_name(const std::string& current_name) const {
			std::string name;

			for (const auto& d : fields) {
				name += d + ".";
			}

			return name + current_name;
		}

		auto get_indent() const {
			return std::string(fields.size() * 4, ' ');
		}
	};
}
