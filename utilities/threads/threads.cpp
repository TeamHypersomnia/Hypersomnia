#pragma once
#include "threads.h"
#include <cassert>

namespace augs {
	namespace threads {
		shared_counter::shared_counter(long sc) : i(sc) {}
		shared_counter& shared_counter::operator++() {
			InterlockedIncrement(&i);
			return *this;
		}
		shared_counter& shared_counter::operator--() {
			InterlockedDecrement(&i);
			return *this;
		}

		int get_num_cores() {
			static SYSTEM_INFO sysinfo;
			GetSystemInfo( &sysinfo );

			return sysinfo.dwNumberOfProcessors;
		}
	}
}