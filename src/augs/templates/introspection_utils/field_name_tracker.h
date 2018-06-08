#pragma once
#include "augs/misc/scope_guard.h"
#include "augs/misc/constant_size_vector.h"

namespace augs {
	class field_name_tracker {
		using string_type = const char*;

		struct entry {
			string_type str;
			int idx;

			entry() {}
			entry(string_type str, int idx) : str(str), idx(idx) {}
		};

		augs::constant_size_vector<entry, 500> fields;

	public:
		void pop() {
			fields.pop_back();
		}

		auto track(const int idx) {
			fields.emplace_back(nullptr, idx);
			return augs::scope_guard([this](){ pop(); });
		}

		auto track(const string_type& name) {
			fields.emplace_back(name, -1);
			return augs::scope_guard([this](){ pop(); });
		}

		void track_no_scope(const int idx) {
			fields.emplace_back(nullptr, idx);
		}

		void track_no_scope(const string_type& name) {
			fields.emplace_back(name, -1);
		}

		auto get_full_name() const {
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

			return name;
		}

		auto get_full_name(const string_type& current_name) const {
			return get_full_name() + current_name;
		}

		auto get_indent() const {
			return std::string(fields.size() * 4, ' ');
		}

		void clear() {
			fields.clear();
		}
	};
}
