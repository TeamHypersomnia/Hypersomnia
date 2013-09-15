#pragma once
#include <unordered_map>

namespace augmentations {
	namespace util {

		/* unordered map wrapper that is used to faciliate binding to lua */
		template<class key, class value>
		class map_wrapper {
			std::unordered_map<key, value> raw_map;
		public:

			void insert(const key& k, const value& v) {
				raw_map.insert(std::make_pair(k, v));
			}

			const value& at(const key& k) const {
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
}