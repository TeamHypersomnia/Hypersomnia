#pragma once

namespace augs {
	namespace entity_system {
		class entity;

		class entity_ptr {
			entity* ptr;
			friend class world;
		public:
			entity_ptr(entity * const ptr = nullptr);
			entity_ptr(const entity_ptr&);
			~entity_ptr();

			void set_ptr(entity*);
			entity* get() const;
			bool exists() const;

			operator entity*() const;
			entity_ptr& operator=(const entity_ptr&) ;
			entity_ptr& operator=(entity*) ;
			entity* operator->();
			entity& operator*();

			/* shortcuts for scripts */
			template <typename component_class>
			component_class* find() {
				return ptr->find();
			}

			template <typename component_type>
			component_type* set(const component_type& object) {
				return ptr->set(object);
			}

			template <typename component_type>
			void remove() {
				ptr->remove();
			}

			void clear();
			std::string get_name();

			template <typename component_type>
			component_type& add(const component_type& object = component_type()) {
				return ptr->add(object);
			}
		};
	}
}