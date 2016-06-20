#pragma once
template<bool> class basic_entity_handle;

typedef basic_entity_handle<false> entity_handle;
typedef basic_entity_handle<true> const_entity_handle;