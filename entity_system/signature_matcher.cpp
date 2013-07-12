#include <algorithm>

#include "signature_matcher.h"
#include "entity.h"
#include "world.h"

namespace augmentations {
	namespace entity_system {
		//signature_matcher::signature_matcher(const type_pack& types) : signature(types) {
		//	std::sort(signature.raw_types.begin(), signature.raw_types.end());
		//}
		//	
		//bool signature_matcher::matches(const signature_matcher& bigger) const {
		//	for(auto i = signature.raw_types.begin(); i != signature.raw_types.end(); ++i)
		//		/* it is sufficient to find only one type not present in bigger signature to tell there will be no match */
		//		if(!std::binary_search(bigger.signature.raw_types.begin(), bigger.signature.raw_types.end(), *i)) return false;
		//	return true;
		//}
			
		//type_pack signature_matcher::get_types() const {
		//	return signature;
		//}
		//
		//bool signature_matcher::matches(const type_pack& bigger) const {
		//	for(auto i = signature.raw_types.begin(); i != signature.raw_types.end(); ++i) 
		//		/* it is sufficient to find only one type not present in bigger signature to tell there will be no match */
		//		if(std::find(bigger.raw_types.begin(), bigger.raw_types.end(), *i) == bigger.raw_types.end()) return false;
		//	return true;
		//}

		signature_matcher_bitset::signature_matcher_bitset(const std::vector<registered_type>& types_with_ids) {
			for(auto i = types_with_ids.begin(); i != types_with_ids.end(); ++i)
				signature.set((*i).id, true);
		}
			
		bool signature_matcher_bitset::matches(const signature_matcher_bitset& bigger) const {
			return (signature & bigger.signature) == signature;
		}
	}
}