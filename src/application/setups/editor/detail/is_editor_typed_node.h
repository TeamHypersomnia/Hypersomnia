#pragma once

template <class E>
struct editor_typed_node_id;

template <class T>
struct is_editor_typed_node_id : std::false_type {};

template <class T>
struct is_editor_typed_node_id<editor_typed_node_id<T>> : std::true_type {};

template <class T>
static constexpr bool is_editor_typed_node_id_v = is_editor_typed_node_id<T>::value;
