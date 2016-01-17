#pragma once
#include "processing_system.h"
#include "component.h"
#include "component_bitset_matcher.h"

#include "misc/sorted_vector.h"
#include "misc/object_pool.h"

#define INCLUDE_COMPONENT_NAMES 1

#define USE_POINTER_TUPLE 1

#ifdef USE_POINTER_TUPLE
#include <tuple>
namespace components {
	struct animation;
	struct animation_response;
	struct behaviour_tree;
	struct camera;
	struct chase;
	struct children;
	struct crosshair;
	struct damage;
	struct gun;
	struct input;
	struct lookat;
	struct movement;
	struct particle_emitter;
	struct particle_group;
	struct pathfinding;
	struct physics;
	struct render;
	struct steering;
	struct transform;
	struct visibility;
	struct sprite;
	struct polygon;
	struct tile_layer;
	struct car;
	struct driver;
	struct trigger;
	struct trigger_detector;
	struct fixtures;
}
#endif

namespace augs {
	class world;

	class entity {
		/* only world class is allowed to instantiate an entity and it has to do it inside object pool */
		friend class type_hash_to_index_mapper;

	public:
		char script_data[sizeof(int) + sizeof(int*)];

		entity(world& owner_world);
		~entity();

		/* maps type hashes into components */
#if USE_POINTER_TUPLE 
		std::tuple<
			std::pair<memory_pool::id, components::animation*>,
			std::pair<memory_pool::id, components::animation_response*>,
			std::pair<memory_pool::id, components::behaviour_tree*>,
			std::pair<memory_pool::id, components::camera*>,
			std::pair<memory_pool::id, components::chase*>,
			std::pair<memory_pool::id, components::children*>,
			std::pair<memory_pool::id, components::crosshair*>,
			std::pair<memory_pool::id, components::damage*>,
			std::pair<memory_pool::id, components::gun*>,
			std::pair<memory_pool::id, components::input*>,
			std::pair<memory_pool::id, components::lookat*>,
			std::pair<memory_pool::id, components::movement*>,
			std::pair<memory_pool::id, components::particle_emitter*>,
			std::pair<memory_pool::id, components::particle_group*>,
			std::pair<memory_pool::id, components::pathfinding*>,
			std::pair<memory_pool::id, components::physics*>,
			std::pair<memory_pool::id, components::render*>,
			std::pair<memory_pool::id, components::steering*>,
			std::pair<memory_pool::id, components::transform*>,
			std::pair<memory_pool::id, components::visibility*>,
			std::pair<memory_pool::id, components::sprite*>,
			std::pair<memory_pool::id, components::polygon*>,
			std::pair<memory_pool::id, components::tile_layer*>,
			std::pair<memory_pool::id, components::car*>,
			std::pair<memory_pool::id, components::driver*>,
			std::pair<memory_pool::id, components::trigger*>,
			std::pair<memory_pool::id, components::trigger_detector*>,
			std::pair<memory_pool::id, components::fixtures*>
		> type_to_component;

		component_bitset_matcher signature;
#else
		sorted_associative_vector<size_t, memory_pool::id> type_to_component;
#endif

#ifdef INCLUDE_COMPONENT_NAMES
		sorted_associative_vector<size_t, std::string> typestrs;
#endif
		std::string name;

		entity_id get_id();

		world& owner_world;

		world& get_owner_world() {
			return owner_world;
		}

		/* removes all components */
		void clear();

		/* get specified component */
		template<class component_class>
		component_class& get() {
			return *find<component_class>();
		}

		template <typename component_class>
		component_class* find() {
#if USE_POINTER_TUPLE
			auto& found = _find<component_class>();
			if (found.alive())
				return (component_class*)found.ptr();
#else
			auto found = type_to_component.get(typeid(component_class).hash_code());
			if (found)
				return reinterpret_cast<component_class*>(found->ptr());
#endif
			return nullptr;
		}

		template <typename component_type>
		component_type* set(const component_type& object) {
			auto* obj = find<component_type>();
			if (obj) {
				(*obj) = object;
			}

			return obj;
		}

		component_bitset_matcher get_component_signature();

		template <typename component_type>
		component_type& add(const component_type& object = component_type()) {
			auto hash = typeid(component_type).hash_code();

			component_bitset_matcher old_signature(get_component_signature());

			if (find<component_type>() != nullptr)
				throw std::exception("component already exists!");


#ifdef INCLUDE_COMPONENT_NAMES
			typestrs.add(hash, typeid(component_type).name());
#endif
#if USE_POINTER_TUPLE 
			auto& component_ptr = *reinterpret_cast<object_pool<component_type>::id*>(&_find<component_type>());
			signature.add(owner_world.component_library.get_index(hash));
#else
			type_to_component.add(hash, memory_pool::id());

			auto& component_ptr = *reinterpret_cast<object_pool<component_type>::id*>(type_to_component.get(hash));
#endif
			/* allocate new component in a corresponding pool */
			component_ptr = owner_world.get_components_by_type<component_type>().allocate(object);

			/* get new signature */
			component_bitset_matcher new_signature(old_signature);
			/* will trigger an exception on debug if the component type was not registered within any existing system */
			new_signature.add(owner_world.component_library.get_index(hash));

			entity_id this_id = get_id();
			for (auto sys : owner_world.get_all_systems())
			{
				bool matches_new = sys->components_signature.matches(new_signature);
				bool doesnt_match_old = !sys->components_signature.matches(old_signature);

				if (matches_new && doesnt_match_old)
					sys->add(this_id);
			}

			return component_ptr.get();
		}
		
		template <typename component_type>
		component_type& operator+= (const component_type& object) {
			return add(object);
		}

	private:
		template <typename component_type>
		void remove() {
#if USE_POINTER_TUPLE
			auto component_type_hash = typeid(component_type).hash_code();

			component_bitset_matcher old_signature(get_component_signature());

			component_bitset_matcher& new_signature = signature;
			signature.remove(owner_world.component_library.get_index(component_type_hash));

			bool is_already_removed = old_signature == new_signature;

			if (is_already_removed)
				return;

			entity_id this_id = get_id();

			for (auto sys : owner_world.get_all_systems())
				/* if a processing_system does not match with the new signature and does with the old one */
				if (!sys->components_signature.matches(new_signature) && sys->components_signature.matches(old_signature))
					/* we should remove this entity from there */
					sys->remove(this_id);

			/* delete component from the corresponding pool, use hash to identify the proper destructor */
			owner_world.get_components_by_hash(typeid(component_type).hash_code()).free_with_destructor(_find<component_type>(), typeid(component_type).hash_code());
			_find<component_type>().unset();

#else
			remove(typeid(component_type).hash_code());

#endif
		}

		void remove(size_t component_type_hash);

#if USE_POINTER_TUPLE 
		template <typename component_class>
		memory_pool::id& _find() {
			return std::get<std::pair<memory_pool::id, component_class*>>(type_to_component).first;
		}
#endif
	};
}

