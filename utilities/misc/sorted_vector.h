#pragma once
#include <vector>
#include <algorithm>

namespace augs {
	template<typename value>
	struct sorted_vector {
		std::vector<value> raw;

		void add(const value& val) {
			raw.push_back(val);
			std::sort(raw.begin(), raw.end());
		}

		bool remove(const value& val) {
			auto it = std::lower_bound(raw.begin(), raw.end(), val);

			if (it != raw.end() && *it == val) {
				raw.erase(it);
				return true;
			}

			return false;
		}

		bool find(const value& key) const {
			return std::binary_search(raw.begin(), raw.end(), key);
		}

		value* get(const value& key) {
			auto it = std::lower_bound(raw.begin(), raw.end(), key);
			if (it == raw.end() || *it != key)
				return nullptr;
			return &*it;
		}
	};

	template<typename key_type, typename value_type>
	struct sorted_associative_vector {
		struct node {
			key_type key;
			value_type val;

			bool operator < (const node& b) const {
				return key < b.key;
			}

			node(const key_type& k, const value_type& v) : key(k), val(v) {}
		};

		std::vector<node> raw;

		void add(const key_type& key, const value_type& val) {
			raw.push_back(node(key, val));
			std::sort(raw.begin(), raw.end());
		}

		bool remove(const key_type& key) {
			auto it = std::lower_bound(raw.begin(), raw.end(), node(key, value_type()));

			if (it != raw.end() && (*it).key == key) {
				raw.erase(it);
				return true;
			}

			return false;
		}

		bool find(const key_type& key) const {
			return std::binary_search(raw.begin(), raw.end(), node(key, value_type()));
		}

		value_type* get(const key_type& key) {
			if (raw.empty()) {
				return nullptr;
			}

			auto it = std::lower_bound(raw.begin(), raw.end(), node(key, value_type()));

			if (it != raw.end()) {
				int index = it - raw.begin();
				node& found = raw[index];

				if (found.key == key)
					return &found.val;
				else
					return nullptr;
			}
			return nullptr;
		}
	};
}