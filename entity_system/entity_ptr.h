#pragma once

namespace augmentations {
	namespace entity_system {
		class entity;

		class entity_ptr {
			entity* ptr;
			friend class world;
		public:
			entity_ptr(entity* ptr = nullptr);
			entity_ptr(const entity_ptr&);
			~entity_ptr();

			operator entity*() const;
			entity_ptr& operator=(const entity_ptr&);
			entity* operator->();
			entity& operator*();
		};
	}
}