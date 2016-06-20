#pragma once
template <bool is_const> class basic_inventory_slot_handle;

typedef basic_inventory_slot_handle<true> const_inventory_slot_handle;
typedef basic_inventory_slot_handle<false> inventory_slot_handle;
