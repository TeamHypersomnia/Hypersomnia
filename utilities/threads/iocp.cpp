#pragma once
#include "threads.h"
#include "../error/error.h"

namespace augs {
	namespace threads {

		completion::completion(overlapped* o, DWORD result, DWORD key) : overlap(o ? &(o->overlap) : 0), result(result), key(key) {}
		
		DWORD& completion::get_key() { return key; }

		DWORD& completion::get_result() { return result; }
		
		void completion::get_overlapped(overlapped*& p) {
			p = (overlapped*)overlap;
		}

		void completion::get_overlapped(augs::network::overlapped*& p) {
			p = (augs::network::overlapped*)overlap;

		}

		void completion::get_overlapped(augs::network::overlapped_accept*& p) {
			p = (augs::network::overlapped_accept*)overlap;

		}


		completion iocp::QUIT;

		iocp::iocp() : cport(0), workers(0) { }

		void iocp::open(int concurrency_value) {
			if(!cport) err((cport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, (cval = concurrency_value))));
		}

		bool iocp::get_completion(completion& out, DWORD ms_timeout) {
			return err(GetQueuedCompletionStatus(cport, &out.get_result(), &out.get_key(), &out.overlap, ms_timeout)) != FALSE;
		}

		bool iocp::post_completion(completion& in) {
			return err(PostQueuedCompletionStatus(cport, in.result, in.key, in.overlap)) != FALSE;
		}

		void iocp::quit_pool() {
			if(!workers) return;
			err(PostQueuedCompletionStatus(cport, 0, 0, 0));
			WaitForMultipleObjects(nworkers, workers, true, INFINITE);
			for(int i = 0; i < nworkers; ++i) 
				err(CloseHandle(workers[i]));

			delete [] workers;
			workers = 0;
		}

		void iocp::close() {
			if(!cport) return;
			quit_pool();
			err(CloseHandle(cport));
			cport = 0;
		}

		iocp::~iocp() {
			close();
		}
	}
}