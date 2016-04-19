#pragma once
#pragma warning(disable : 4503)
#include <type_traits>

#include "processing_system.h"
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
	struct position_copying;
	struct crosshair;
	struct damage;
	struct gun;
	struct input_receiver;
	struct rotation_copying;
	struct movement;
	struct particle_effect_response;
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
	struct trigger_query_detector;
	struct fixtures;
	struct container;
	struct item;
	struct force_joint;
	struct physics_definition;
	struct item_slot_transfers;
	struct gui_element;
	struct trigger_collision_detector;
	struct name;
	struct trace;
}
#endif

class destroy_system;
namespace augs {
	class world;

	class entity {
		friend class world;
		friend class ::destroy_system;
		friend class memory_pool::typed_id_template<entity>;

		std::unordered_map<sub_definition_name, augs::entity_id> sub_definitions;
		std::unordered_map<sub_entity_name, augs::entity_id> sub_entities_by_name;
		std::unordered_map<associated_entity_name, augs::entity_id> associated_entities_by_name;

		std::vector<augs::entity_id> sub_entities;
		augs::entity_id parent;
		augs::entity_id self_id;
		sub_entity_name name_as_sub_entity = sub_entity_name::INVALID;
		sub_definition_name name_as_sub_definition = sub_definition_name::INVALID;

		bool born_as_definition_entity = false;
	public:
		entity(world& owner_world);
		~entity();

		bool is_definition_entity();
		sub_entity_name get_name_as_sub_entity();
		sub_definition_name get_name_as_sub_definition();

		augs::entity_id get_parent();
		void add_sub_entity(augs::entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID);
		void map_sub_entity(sub_entity_name n, augs::entity_id p);
		void map_sub_definition(sub_definition_name n, augs::entity_id p);
		void for_each_sub_entity(std::function<void(augs::entity_id)>);
		void for_each_sub_definition(std::function<void(augs::entity_id)>);

		/* maps type hashes into components */
#if USE_POINTER_TUPLE 
		std::tuple<
			std::pair<memory_pool::id, components::animation*>,
			std::pair<memory_pool::id, components::animation_response*>,
			std::pair<memory_pool::id, components::behaviour_tree*>,
			std::pair<memory_pool::id, components::camera*>,
			std::pair<memory_pool::id, components::position_copying*>,
			std::pair<memory_pool::id, components::crosshair*>,
			std::pair<memory_pool::id, components::damage*>,
			std::pair<memory_pool::id, components::gun*>,
			std::pair<memory_pool::id, components::input_receiver*>,
			std::pair<memory_pool::id, components::rotation_copying*>,
			std::pair<memory_pool::id, components::movement*>,
			std::pair<memory_pool::id, components::particle_effect_response*>,
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
			std::pair<memory_pool::id, components::trigger_query_detector*>,
			std::pair<memory_pool::id, components::fixtures*>,
			std::pair<memory_pool::id, components::container*>,
			std::pair<memory_pool::id, components::force_joint*>,
			std::pair<memory_pool::id, components::item*>,
			std::pair<memory_pool::id, components::physics_definition*>,
			std::pair<memory_pool::id, components::item_slot_transfers*>,
			std::pair<memory_pool::id, components::gui_element*>,
			std::pair<memory_pool::id, components::trigger_collision_detector*>,
			std::pair<memory_pool::id, components::name*>,
			std::pair<memory_pool::id, components::trace*>
		> type_to_component;

		component_bitset_matcher signature;
#else
		sorted_associative_vector<size_t, memory_pool::id> type_to_component;
#endif

#ifdef INCLUDE_COMPONENT_NAMES
		sorted_associative_vector<size_t, std::string> typestrs;
#endif
		std::string debug_name;

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
		void enable(const component_type& object = component_type()) {
			add_to_compatible_systems(typeid(std::remove_pointer<std::decay<component_type>::type>::type).hash_code());
		}

		template <typename component_type>
		void disable(const component_type& object = component_type()) {
			unplug_component_from_systems(typeid(std::remove_pointer<std::decay<component_type>::type>::type).hash_code());
		}

		template <typename component_type>
		bool is_enabled(const component_type& object = component_type()) {
			return signature.is_set(owner_world.component_library.get_index(typeid(component_type).hash_code()));
		}

		template <typename component_type>
		component_type& add(const component_type& object = component_type()) {
			if (find<component_type>() != nullptr)
				throw std::exception("component already exists!");

			auto hash = typeid(component_type).hash_code();

#ifdef INCLUDE_COMPONENT_NAMES
			typestrs.add(hash, typeid(component_type).name());
#endif
#if USE_POINTER_TUPLE 
			auto& component_ptr = *reinterpret_cast<object_pool<component_type>::typed_id*>(&_find<component_type>());
#else
			type_to_component.add(hash, memory_pool::id());

			auto& component_ptr = *reinterpret_cast<object_pool<component_type>::typed_id*>(type_to_component.get(hash));
#endif
			/* allocate new component in a corresponding pool */
			component_ptr = owner_world.get_components_by_type<component_type>().allocate(object);
			
			add_to_compatible_systems(hash);

			return component_ptr.get();
		}

		template <typename component_type>
		component_type& operator+= (const component_type& object) {
			return add(object);
		}

		template <typename component_type>
		void remove() {
#if USE_POINTER_TUPLE
			if (!unplug_component_from_systems(typeid(component_type).hash_code()))
				return;

			/* delete component from the corresponding pool, use hash to identify the proper destructor */
			owner_world.get_components_by_hash(typeid(component_type).hash_code()).free_with_destructor(_find<component_type>(), typeid(component_type).hash_code());
			_find<component_type>().unset();

#else
			remove(typeid(component_type).hash_code());
#endif
		}

		void remove(size_t component_type_hash);

	private:
		void clone(augs::entity_id);

		bool unplug_component_from_systems(size_t);
		void add_to_compatible_systems(size_t);

#if USE_POINTER_TUPLE 
		template <typename component_class>
		memory_pool::id& _find() {
			return std::get<std::pair<memory_pool::id, component_class*>>(type_to_component).first;
		}
#endif
	};
}

