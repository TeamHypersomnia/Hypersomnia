#pragma once

std::string format_field_name(std::string s);

template <class List>
class type_in_list_id;

template <class T>
std::string get_type_name();

template <class T>
std::string get_type_name(const T&);

template <class T>
std::string get_type_name(const type_in_list_id<T>& id);
