#pragma once
template<bool> class basic_inventory_slot_handle;

typedef basic_inventory_slot_handle<false> inventory_slot_handle;
typedef basic_inventory_slot_handle<true> const_inventory_slot_handle;
