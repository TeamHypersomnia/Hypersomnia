#include "stdafx.h"
#include <algorithm>

#include "signature_matcher.h"
#include "entity.h"
#include "world.h"

namespace augs {
	namespace entity_system {
		signature_matcher_bitset::signature_matcher_bitset(const std::vector<registered_type>& types_with_ids) {
			add(types_with_ids);
		}
			
		void signature_matcher_bitset::add(const registered_type& r) {
			signature.set(r.id, true);
		}

		void signature_matcher_bitset::remove(const registered_type& r) {
			signature.set(r.id, false);
		}

		void signature_matcher_bitset::add(const std::vector<registered_type>& types_with_ids) {
			for(auto i = types_with_ids.begin(); i != types_with_ids.end(); ++i)
				add(*i);
		}
			
		void signature_matcher_bitset::remove(const std::vector<registered_type>& types_with_ids) {
			for(auto i = types_with_ids.begin(); i != types_with_ids.end(); ++i)
				remove(*i);
		}
			
		bool signature_matcher_bitset::operator==(const signature_matcher_bitset& b) const {
			return signature == b.signature;
		}

		bool signature_matcher_bitset::matches(const signature_matcher_bitset& bigger) const {
			return (signature & bigger.signature) == signature;
		}
	}
}