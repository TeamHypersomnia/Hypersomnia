#pragma once

template <class E>
struct editor_typed_resource_id;

template <class T>
struct is_editor_typed_resource_id : std::false_type {};

template <class T>
struct is_editor_typed_resource_id<editor_typed_resource_id<T>> : std::true_type {};

template <class T>
static constexpr bool is_editor_typed_resource_id_v = is_editor_typed_resource_id<T>::value;
