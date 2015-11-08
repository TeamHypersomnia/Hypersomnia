#pragma once
#include <unordered_map>

namespace augs {
	/* unordered map wrapper that is used to faciliate binding to lua */
	template<class key, class value>
	class map_wrapper {
		std::unordered_map<key, value> raw_map;
	public:
		void add(key k, value v) {
			raw_map[k] = v;
		}

		void insert(const key& k, const value& v) {
			raw_map[k] = v;
		}

		void remove(const key& k) {
			raw_map.erase(k);
		}

		value& at(const key& k) {
			return raw_map.at(k);
		}

		struct find_result {
			bool found;
			value* written_value;

			value& get_value() {
				return *written_value;
			}
		};

		find_result find(const key& k) {
			find_result out;

			auto found = raw_map.find(k);

			if (found == raw_map.end()) {
				out.found = false;
				out.written_value = nullptr;
			}
			else {
				out.found = true;
				out.written_value = &found->second;
			}

			return out;
		}

		value& get(key k) {
			return raw_map.at(k);
		}

		value& operator [](const key& k) {
			return raw_map[k];
		}

		size_t size() const {
			return raw_map.size();
		}

		std::unordered_map<key, value>& get_raw() {
			return raw_map;
		}
	};
}