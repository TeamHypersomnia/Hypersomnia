#pragma once

template <class T, class = void>
struct has_affected_entities : std::false_type {};

template <class T>
struct has_affected_entities<T, decltype(std::declval<T&>().affected_entities, void())> : std::true_type {};

template <class T>
constexpr bool has_affected_entities_v = has_affected_entities<T>::value;


template <class T, class K, class = void>
struct has_member_sanitize : std::false_type {};

template <class T, class K>
struct has_member_sanitize<T, K, decltype(std::declval<T&>().sanitize(std::declval<const K&>()), void())> : std::true_type {};

template <class T, class K>
constexpr bool has_member_sanitize_v = has_member_sanitize<T, K>::value;
