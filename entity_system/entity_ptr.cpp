#include "entity_ptr.h"
#include "entity.h"

namespace augmentations {
	namespace entity_system {
		entity_ptr::entity_ptr(const entity_ptr& p) : entity_ptr(p.ptr) {}

		entity_ptr::entity_ptr(entity* ptr) : ptr(ptr) {
			if (ptr)
				ptr->owner_world.register_entity_watcher(*this);
		}

		entity_ptr::~entity_ptr() {
			if (ptr)
				ptr->owner_world.unregister_entity_watcher(*this);
		}

		entity_ptr::operator entity*() const {
			return ptr;
		}

		entity_ptr& entity_ptr::operator=(const entity_ptr& p) {
			if (ptr)
				ptr->owner_world.unregister_entity_watcher(*this);
			
			ptr = p.ptr;

			if (ptr)
				ptr->owner_world.register_entity_watcher(*this);

			return *this;
		}

		entity* entity_ptr::operator->() {
			return ptr;
		}

		entity& entity_ptr::operator*() {
			return *ptr;
		}
	}
}