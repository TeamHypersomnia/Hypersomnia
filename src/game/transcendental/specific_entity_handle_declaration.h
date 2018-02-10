#pragma once

template <class>
struct empty_id_provider;

template <class derived_handle_type>
struct iterated_id_provider;

template <class derived_handle_type>
struct stored_id_provider;

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
using basic_noid_entity_handle = specific_entity_handle<C, E, empty_id_provider>;

template <class entity_type>
using noid_entity_handle = basic_noid_entity_handle<false, entity_type>;

template <class entity_type>
using const_noid_entity_handle = basic_noid_entity_handle<true, entity_type>;

/* Shortcut */

template <class T>
using entity_type_of = typename std::decay_t<T>::used_entity_type;
