#pragma once
#include "templated_list.h"
#include <algorithm>

namespace augmentations {
	namespace entity_system {
		void get_types(templated_list<>, std::vector<base_type>&) {}

		void type_pack::add_n(const type_pack& adder) {
			/* just concatenate two vectors */
			for(auto it = adder.raw_types.begin(); it != adder.raw_types.end(); ++it)
				raw_types.push_back(*it);
		}

		void type_pack::remove_n(const type_pack& remover) {
			/* sort for finding efficiency */
			auto sorted_remover = remover;
			std::sort(sorted_remover.raw_types.begin(), sorted_remover.raw_types.end());

			/* delete every processing_system found on remover's list */
			raw_types.erase(std::remove_if(raw_types.begin(), raw_types.end(), [&remover](const base_type& it) {
				return !std::binary_search(remover.raw_types.begin(), remover.raw_types.end(), it);
			}), raw_types.end());
		}
	}
}