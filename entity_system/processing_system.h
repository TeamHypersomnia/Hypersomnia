#pragma once
#include <vector>
#include <functional>
#include "signature_matcher.h"

namespace augmentations {
	namespace entity_system {
		class processing_system {
			friend class world;
			friend class entity;
			std::vector<entity*> targets;
			
			signature_matcher_bitset components_signature;
		public:
			
			/* add an entity to processing_system 
			base function just push_backs entity to targets
			*/
			virtual void add(entity*);
			/* remove entity from processing_system 
			base function just removes entity from targets

			note: there's no sense in introducing remove_n function as complexity using "remove_if" once is the same as using "remove" multiple times
			*/
			virtual void remove(entity*);

			/* process all entity targets */
			virtual void process_entities() = 0;

			/* you are required to override this function to specify components that this system needs to processing */
			virtual type_pack get_needed_components() const = 0;

			/* "targets" read-only getter */
			const std::vector<entity*>& get_targets() const;

			/* helper function that iterates through every target entity */
			void for_each(std::function<void (entity*)>);
		};

		/* helper class removing necessity to override get_needed_components by specifying the types in the parameter pack */
		template<typename... needed_components>
		class processing_system_templated : public processing_system {
			virtual type_pack get_needed_components() const override {
				return templated_list<needed_components...>::get();
			}
		};
	}
}