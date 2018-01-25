

namespace entity_types {
	using character = augs::trivially_copyable_tuple<
		invariants::rigid_body,
		invariants::fixtures
	>
}

struct entity_id : public cosmic_object_pool_id<cosmic_entity> {
	using base = cosmic_object_pool_id<cosmic_entity>;
	// GEN INTROSPECTOR struct entity_id
	// INTROSPECT BASE cosmic_object_pool_id<cosmic_entity>
	// END GEN INTROSPECTOR

	entity_id(const child_entity_id c);
	entity_id(const base b = base()) : base(b) {}

	operator unversioned_entity_id() const {
		return static_cast<unversioned_entity_id::base>(*static_cast<const base*>(this));
	}
};

template <class entity_type>
struct typed_entity_id : public cosmic_object_pool_id<cosmic_entity> {
	using base = cosmic_object_pool_id<cosmic_entity>;

	// GEN INTROSPECTOR struct typed_entity_id class entity_type
	// INTROSPECT BASE cosmic_object_pool_id<cosmic_entity>
	// END GEN INTROSPECTOR

	typed_entity_id(const child_entity_id c);
	typed_entity_id(const base b = base()) : base(b) {}

	operator entity_id() const {

	}
}

using entity_type_id = type_in_list_id<entity_type_list_t<type_list>>;

using basic_flavour_id = unsigned;

struct entity_flavour_id {
	entity_type_id type_id;
	basic_flavour_id flavour_id;
};

template <class T>
struct typed_entity_flavour_id {
	basic_flavour_id flavour_id;
};	

template <bool is_const, class entity_type, class... accessed_components>
class typed_entity_handle {
	static constexpr constrained_access = sizeof...(accessed_components) > 0;

	template <class = std::enable_if_t<!constrained_access>>
	operator basic_entity_handle<is_const>() const {

	}
};

