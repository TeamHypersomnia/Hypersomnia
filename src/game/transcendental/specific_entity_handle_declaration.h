#pragma once

template <class>
struct empty_id_provider;

template <class derived_handle_type>
struct iterated_id_provider;

template <class derived_handle_type>
struct stored_id_provider;

template <class derived_handle_type>
struct ref_stored_id_provider;


template <bool is_const, class entity_type, template <class> class identifier_provider>
class specific_entity_handle;


template <bool C, class E>
using basic_typed_entity_handle = specific_entity_handle<C, E, stored_id_provider>;

template <class entity_type>
using typed_entity_handle = basic_typed_entity_handle<false, entity_type>;

template <class entity_type>
using const_typed_entity_handle = basic_typed_entity_handle<true, entity_type>;


template <bool C, class E>
using basic_iterated_entity_handle = specific_entity_handle<C, E, iterated_id_provider>;

template <class entity_type>
using iterated_entity_handle = basic_iterated_entity_handle<false, entity_type>;

template <class entity_type>
using const_iterated_entity_handle = basic_iterated_entity_handle<true, entity_type>;


template <bool C, class E>
using basic_ref_typed_entity_handle = specific_entity_handle<C, E, ref_stored_id_provider>;

template <class entity_type>
using ref_typed_entity_handle = basic_ref_typed_entity_handle<false, entity_type>;

template <class entity_type>
using cref_typed_entity_handle = basic_ref_typed_entity_handle<true, entity_type>;

#if TODO
template <bool C, class E>
using basic_noid_entity_handle = specific_entity_handle<C, E, empty_id_provider>;

template <class entity_type>
using noid_entity_handle = basic_noid_entity_handle<false, entity_type>;

template <class entity_type>
using const_noid_entity_handle = basic_noid_entity_handle<true, entity_type>;
#endif

/* Shortcut */

template <class T>
struct detail_entity_type_of {
	using type = typename std::decay_t<T>::used_entity_type;
};

template <bool A, class B, template <class> class C>
struct detail_entity_type_of<specific_entity_handle<A, B, C>> {
	using type = B;
};

template <class T>
using entity_type_of = typename detail_entity_type_of<T>::type;
