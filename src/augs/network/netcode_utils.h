#pragma once
#include <string>

struct netcode_address_t;

std::string ToString(const netcode_address_t&);
bool operator==(const netcode_address_t& a, const netcode_address_t& b);
bool operator!=(const netcode_address_t& a, const netcode_address_t& b);

bool try_fire_interval(double interval, net_time_t& when_last);
bool try_fire_interval(double interval, net_time_t& when_last, double current_time);
