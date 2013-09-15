#include "entity_ptr.h"
#include "entity.h"

namespace augmentations {
	namespace entity_system {
		entity_ptr::entity_ptr(const entity_ptr& p) : entity_ptr(p.ptr) {}

		entity_ptr::entity_ptr(entity * const ptr) : ptr(ptr) {
			if (ptr)
				ptr->owner_world.register_entity_watcher(*this);
		}

		entity_ptr::~entity_ptr() {
			if (ptr)
				ptr->owner_world.unregister_entity_watcher(*this);
		}

		void entity_ptr::set(entity* p) {
			if (ptr)
				ptr->owner_world.unregister_entity_watcher(*this);

			ptr = p;

			if (ptr)
				ptr->owner_world.register_entity_watcher(*this);
		}

		entity* entity_ptr::get() const {
			return ptr;
		}

		entity_ptr::operator entity*() const {
			return ptr;
		}

		entity_ptr& entity_ptr::operator=(const entity_ptr& p) {
			set(p.ptr);
			return *this;
		}

		entity* entity_ptr::operator->() {
			return get();
		}

		entity& entity_ptr::operator*() {
			return *get();
		}
	}
}