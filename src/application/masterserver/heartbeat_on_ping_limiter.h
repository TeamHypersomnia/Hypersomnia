#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>
#include "augs/templates/container_templates.h"
#include "augs/network/netcode_utils.h"

/*
	Who gets a heartbeat to a ping - so that the server can not be used to flood anyone with them.

	A few addresses at a time (more from the local network), each until it goes quiet for a while;
	each with a bucket that lets a burst of refreshes through and fills up again by one a second.
	All taken or the bucket empty: only the ping answers, as for the server list's servers.
	At most about 26 KB/s from the internet per server instance, whatever comes in.
*/

class heartbeat_on_ping_limiter {
	static constexpr std::size_t max_internet_addresses_v = 10;
	static constexpr std::size_t max_local_addresses_v = 64;

	static constexpr double forget_after_secs_v = 15.0;
	static constexpr double burst_v = 5.0;
	static constexpr double refill_per_sec_v = 1.0;

	struct requester {
		netcode_address_t address;
		double last_at = 0.0;
		double tokens = 0.0;
	};

	std::vector<requester> internet;
	std::vector<requester> local;

public:
	bool allow(const netcode_address_t& from, const double now, const bool from_local_network) {
		auto& requesters = from_local_network ? local : internet;
		const auto max_addresses = from_local_network ? max_local_addresses_v : max_internet_addresses_v;

		for (auto& r : requesters) {
			if (r.address == from) {
				r.tokens = std::min(burst_v, r.tokens + (now - r.last_at) * refill_per_sec_v);
				r.last_at = now;

				if (r.tokens >= 1.0) {
					r.tokens -= 1.0;
					return true;
				}

				return false;
			}
		}

		erase_if(requesters, [&](const requester& r) {
			return now - r.last_at > forget_after_secs_v;
		});

		if (requesters.size() >= max_addresses) {
			return false;
		}

		requesters.push_back({ from, now, burst_v - 1.0 });
		return true;
	}
};
