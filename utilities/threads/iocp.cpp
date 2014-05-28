#pragma once
#include "stdafx.h"
#include "threads.h"
#include "../error/error.h"

namespace augs {
	namespace threads {

		completion::completion(overlapped* o, DWORD result, DWORD key) : overlap(o ? &(o->overlap) : 0), result(result), key(key) {}
		
		DWORD& completion::get_key() { return key; }

		DWORD& completion::get_result() { return result; }
		
		void completion::get_operation_info(overlapped*& p) {
			p = (overlapped*)overlap;
		}

		void completion::get_operation_info(augs::network::overlapped*& p) {
			p = (augs::network::overlapped*)overlap;

		}

		void completion::get_operation_info(augs::network::overlapped_accept*& p) {
			p = (augs::network::overlapped_accept*)overlap;

		}

		completion iocp::QUIT;

		void overlapped_userdata::on_completion(overlapped* owner) {

		}

		iocp::iocp() : cport(0) {}

		void iocp::open(int concurrency_value) {
			if(!cport) err((cport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, (cval = concurrency_value))));
		}

		bool iocp::get_completion(completion& out, DWORD ms_timeout) {
			return err(GetQueuedCompletionStatus(cport, &out.get_result(), &out.get_key(), &out.overlap, ms_timeout)) != FALSE;
		}

		bool iocp::post_completion(completion& in) {
			return err(PostQueuedCompletionStatus(cport, in.result, in.key, in.overlap)) != FALSE;
		}

		void iocp::add_worker(std::function<void()>* worker_function) {
			HANDLE new_worker;
			err(new_worker = CreateThread(0, 0, ThreadProc, (LPVOID) worker_function, 0, 0));
			worker_handles.push_back(new_worker);
		}

		void iocp::quit_pool() {
			if(!worker_handles.empty()) return;
			err(PostQueuedCompletionStatus(cport, 0, 0, 0));

			WaitForMultipleObjects(worker_handles.size(), worker_handles.data(), true, INFINITE);
			for (int i = 0; i < worker_handles.size(); ++i)
				err(CloseHandle(worker_handles[i]));
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