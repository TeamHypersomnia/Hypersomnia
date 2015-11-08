#pragma once
#include <set>
#include <bitset>
#include "type_hash_to_index_mapper.h"

#define MAXIMUM_COMPONENTS 64

namespace augs {
	namespace entity_system {
		class world;

		/* used to detect whenever a certain system needs to process given entity */
		class component_bitset_matcher {
			std::bitset<MAXIMUM_COMPONENTS> signature;
		public:
			component_bitset_matcher(std::vector<unsigned> indices = std::vector<unsigned>());
			
			void add(unsigned);
			void add(std::vector<unsigned>);
			void remove(unsigned);
			void remove(std::vector<unsigned>);

			bool operator==(const component_bitset_matcher&) const;
			bool matches(const component_bitset_matcher& bigger) const;
		};
	}
}