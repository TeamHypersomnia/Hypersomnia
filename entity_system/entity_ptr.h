#pragma once

namespace augmentations {
	namespace entity_system {
		class entity;

		class entity_ptr {
			entity* ptr;
			friend class world;
		public:
			entity_ptr(entity * const ptr = nullptr);
			entity_ptr(const entity_ptr&);
			~entity_ptr();

			void set(entity*);
			entity* get() const;
			bool exists() const;

			operator entity*() const;
			entity_ptr& operator=(const entity_ptr&) ;
			entity_ptr& operator=(entity*) ;
			entity* operator->();
			entity& operator*();
		};
	}
}