#pragma once
#include <map>
#include <forward_list>
#include "../utility/type_mapper.h"
#include "component.h"
#include "signature_matcher.h"
#define BOOST_DISABLE_THREADS
#include <boost\pool\object_pool.hpp>

namespace augmentations {
	namespace entity_system {
		class world;
		class system;

		class entity : templated_add<world&>, templated_remove<world&> {
			/* only world class is allowed to instantiate an entity */
			friend class world;
			friend class boost::object_pool<entity>;
			
			entity();
			~entity();

			/* maps type hashes into components */
			util::type_mapper<std::map<type_hash, component*>> type_to_component;

			std::vector<system*> get_interested_systems(world&);
		public:
			/* returns type_to_component */
			const augmentations::util::type_mapper<std::map<type_hash, component*>>& raw_mapper() const;

			/* get information about component types from the entity */
			std::vector<registered_type> get_components(const world&) const;
			
			/* sums components specified in component_types with current components of the entity */
			void add(const type_pack& component_types, world& owner_world) override;
			/* removes components specified in component_types from current components of the entity */
			void remove(const type_pack& component_types, world& owner_world) override;

			/* removes all components */
			void clear(world& owner_world);

			/* get value of a component of component_class from the entity */
			template<class component_class>
			component_class& get() {
				return components.get<component_class>();
			}
		};
	}
}