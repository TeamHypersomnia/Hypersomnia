#include <algorithm>

#include "component_bitset_matcher.h"
#include "entity.h"
#include "world.h"

namespace augs {
	namespace entity_system {
		component_bitset_matcher::component_bitset_matcher(std::vector<unsigned> types_with_ids) {
			add(types_with_ids);
		}

		void component_bitset_matcher::add(unsigned index) {
			signature.set(index, true);
		}

		void component_bitset_matcher::remove(unsigned index) {
			signature.set(index, false);
		}

		void component_bitset_matcher::add(std::vector<unsigned> indices) {
			for (int i : indices)
				add(i);
		}

		void component_bitset_matcher::remove(std::vector<unsigned> indices) {
			for (int i : indices)
				remove(i);
		}

		bool component_bitset_matcher::operator==(const component_bitset_matcher& b) const {
			return signature == b.signature;
		}

		bool component_bitset_matcher::matches(const component_bitset_matcher& bigger) const {
			return (signature & bigger.signature) == signature;
		}
	}
}