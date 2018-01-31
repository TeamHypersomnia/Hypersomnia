#pragma once
struct entity_id;

template <class T>
struct basic_item_slot_transfer_request;

using item_slot_transfer_request = basic_item_slot_transfer_request<entity_id>;