#pragma once
#define UNICODE
#include <Windows.h>
#include <vector>

#include "../error/error.h"

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

		class overlapped {
			friend class completion;
			friend class augs::network::udp;
			friend class augs::network::tcp;

			OVERLAPPED overlap;
			DWORD result;
		public:

			class multiple_waits {
				HANDLE* events;
				int n;
				multiple_waits(const multiple_waits&); // disable copy
			public:
				multiple_waits(overlapped*, int cnt);
				~multiple_waits();
				bool wait(DWORD timeout, bool all);
			};

			overlapped(); ~overlapped();
			void create_event();
			bool wait(DWORD timeout = INFINITE);
			int get_result() const;
		};
			
		class completion {
			friend class iocp;
			OVERLAPPED* overlap;
			DWORD result, key;
		public:
			completion(overlapped* o = 0, DWORD result = 0, DWORD key = 0);

			template<class T>
			bool get_user(T*& p) {
				return (p = (T*)overlap) != 0;
			}

			DWORD& get_key(), &get_result();
			
			void get_overlapped(overlapped*&);
			void get_overlapped(augs::network::overlapped*&);
			void get_overlapped(augs::network::overlapped_accept*&);
		};

		class iocp {
			HANDLE cport, *workers;
			int cval, nworkers;
			void* argp, *workerf;

			template<class T>
			static DWORD WINAPI ThreadProc(LPVOID p) {
				iocp* io = (iocp*)p;
				((int (*)(T*))(io->workerf))((T*)(io->argp));
				return 0;
			}
		public:
			iocp();

			static completion QUIT;

			void open(int concurrency_value = get_num_cores());
	
			bool associate(augs::network::udp&, int key);
			bool associate(augs::network::tcp&, int key);

			bool get_completion(completion& out, DWORD ms_timeout = INFINITE);
			bool post_completion(completion& in);

			template <class argument>
			void create_pool(int (*worker)(argument*), argument* arg, int n = get_num_cores()) {
				if(workers || !n) return;
				argp = (void*)arg;
				workerf = (void*)worker;
				nworkers = n;
				workers = new HANDLE[n];
				for(int i = 0; i < n; ++i)
					err(workers[i] = CreateThread(0, 0, ThreadProc<argument>, this, 0, 0));
			}

			void quit_pool(void);
			
			void close();
			~iocp();
		};
	}
}