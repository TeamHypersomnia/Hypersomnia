#pragma once
#include "threads.h"
#include "../error/error.h"

namespace augs {
	namespace threads {
		overlapped::multiple_waits::multiple_waits(overlapped* t, int cnt) {
			events = new HANDLE[n = cnt];
			for(int i = 0; i < n; ++i) events[i] = t[i].overlap.hEvent;
		}

		overlapped::multiple_waits::~multiple_waits() {
			delete [] events;
		}

		bool overlapped::multiple_waits::wait(DWORD timeout, bool all) {
			return err(WaitForMultipleObjects(n, events, all, timeout) != WAIT_FAILED) != 0; 
		}

		overlapped::overlapped() : result(0) {
			SecureZeroMemory(&overlap, sizeof(overlap));
		}

		overlapped::~overlapped() {
			if(overlap.hEvent) err(CloseHandle(overlap.hEvent));
			overlap.hEvent = 0;
		}

		void overlapped::create_event() {
			if(!overlap.hEvent) err((overlap.hEvent = CreateEvent(NULL, FALSE, TRUE, NULL)));
		}

		bool overlapped::wait(DWORD timeout) { 
			return err(WaitForMultipleObjects(1, &overlap.hEvent, TRUE, timeout) != WAIT_FAILED) != 0; 
		}

		int overlapped::get_result() const {
			return result;
		}
	}
}