#pragma once
struct item_slot_transfer_request_data;

template <bool C>
struct basic_item_slot_transfer_request;

typedef basic_item_slot_transfer_request<false> item_slot_transfer_request;
typedef basic_item_slot_transfer_request<true> const_item_slot_transfer_request;
