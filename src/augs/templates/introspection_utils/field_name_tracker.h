#pragma once
#include "augs/misc/scope_guard.h"

namespace augs {
	class field_name_tracker {
		using string_type = const char*;

		struct entry {
			string_type str;
			int idx;
		};

		std::vector<entry> fields;

		void pop() {
			fields.pop_back();
		}

	public:
		auto track() {
			return true;
		}

		auto track(const int idx) {
			fields.push_back(entry { nullptr, idx });
			return augs::scope_guard([this](){ pop(); });
		}

		auto track(const string_type& name) {
			fields.push_back({ name, -1 });
			return augs::scope_guard([this](){ pop(); });
		}

		auto get_full_name(const string_type& current_name) const {
			std::string name;

			for (const auto& d : fields) {
				if (d.idx != -1) {
					name += std::to_string(d.idx);
				}
				else {
					name += d.str;
				}

				name +=	".";
			}

			return name + current_name;
		}

		auto get_indent() const {
			return std::string(fields.size() * 4, ' ');
		}

		void clear() {
			fields.clear();
		}
	};
}
