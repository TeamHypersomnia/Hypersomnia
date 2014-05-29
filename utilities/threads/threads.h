#pragma once
#include "../error/error.h"
#include <memory>

/* todo: rozbudowac pool o priority i critical path */

namespace augs  {
	namespace network {
		class udp;
		class tcp;
		struct overlapped;
		class overlapped_accept;
	}
}

namespace augs {
	namespace threads {
		int get_num_cores();

		struct shared_counter {
			long i;
			shared_counter(long i = 0);
			shared_counter& operator+=(long);
			shared_counter& operator++();
			shared_counter& operator--();
		};

		class overlapped;
		struct overlapped_userdata {
			virtual void on_completion(overlapped* owner);
		};

		class overlapped {
			friend class completion;
			friend class augs::network::udp;
			friend class augs::network::tcp;
		public:
			OVERLAPPED overlap;
			DWORD result;

			std::unique_ptr<overlapped_userdata> userdata;

			class multiple_waits {
				HANDLE* events;
				int n;
				multiple_waits(const multiple_waits&); // disable copy
			public:
				multiple_waits(overlapped*, int cnt);
				~multiple_waits();
				bool wait(DWORD timeout, bool all);
			};

			overlapped(overlapped_userdata* new_userdata = nullptr); ~overlapped();
			
			void reset();

			void create_event();
			bool wait(DWORD timeout = INFINITE);
		};
			
		

		class completion {
			friend class iocp;
			OVERLAPPED* overlap;
			DWORD result, key;
		public:
			enum result {
				SUCCESSFUL_IO_OP_OR_CUSTOM, /* returns true and non-null overlapped struct */
				FAILED_IO_OPERATION, /* returns false and non-null overlapped struct */
				FAILED_TO_DEQUEUE_OR_TIMEOUT,  /* returns false and null overlapped struct */
				CUSTOM_POST /* returns true and null overlapped struct */
			} result_type;

			completion(overlapped* o = 0, DWORD result = 0, DWORD key = 0);

			template<class T>
			bool get_userdata_from_overlapped(T*& target_userdata_object) {
				return (target_userdata_object = (T*) overlap) != 0;
			}

			DWORD& get_key(), &get_result();
			
			void get_operation_info(overlapped*&);
			void get_operation_info(augs::network::overlapped*&);
			void get_operation_info(augs::network::overlapped_accept*&);
		};

		class iocp {
			HANDLE cport;
			std::vector<HANDLE> worker_handles;
			std::vector<std::function<void()>> worker_functions; // just a container

			int cval;
		public:
			static DWORD WINAPI ThreadProc(LPVOID arg) {
				(*((std::function<void()>*)arg))();
				return 0;
			}

			iocp();

			static completion QUIT;

			void open(int concurrency_value = get_num_cores());
	
			bool associate(augs::network::udp&, int key);
			bool associate(augs::network::tcp&, int key);

			completion get_completion(DWORD ms_timeout = INFINITE);
			bool post_completion(completion& in);

			void add_worker(std::function<void()>* worker_function);
			void quit_pool(void);
			
			void close();
			~iocp();
		};
	}
}