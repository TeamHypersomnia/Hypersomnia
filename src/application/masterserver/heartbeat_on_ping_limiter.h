#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>
#include "augs/templates/container_templates.h"
#include "augs/network/netcode_utils.h"

/*
	Who gets a heartbeat to a ping - so that the server can not be used to flood anyone with them.

	Many addresses at a time (more from the local network), each until it goes quiet for a while;
	each with a bucket that lets a burst of refreshes through and fills up again by one a second.
	All taken or the bucket empty: only the ping answers, as for the server list's servers.
	A request is padded to 1200 bytes (min_heartbeat_request_bytes_v), so an answer is only about twice it -
	the limits are about the server's upload, not about floods. At most half the places per IP from the internet:
	one address can't take them all by asking from many ports, yet a club room or a carrier's NAT fits.
*/

class heartbeat_on_ping_limiter {
	static constexpr std::size_t max_internet_addresses_v = 64;
	static constexpr std::size_t max_local_addresses_v = 256;
	static constexpr std::size_t max_internet_ports_per_ip_v = 32;

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

		if (!from_local_network) {
			auto ip_of = [](netcode_address_t a) {
				a.port = 0;
				return a;
			};

			const auto from_this_ip = std::count_if(requesters.begin(), requesters.end(), [&](const requester& r) {
				return ip_of(r.address) == ip_of(from);
			});

			if (static_cast<std::size_t>(from_this_ip) >= max_internet_ports_per_ip_v) {
				return false;
			}
		}

		requesters.push_back({ from, now, burst_v - 1.0 });
		return true;
	}
};
