#pragma once
#include "stdafx.h"
#include "network.h"
#include "../error/error.h"

namespace augs {
	namespace network {
		WSADATA wsaData;
		
		LPFN_ACCEPTEX tcp::acceptex = NULL;
		LPFN_CONNECTEX tcp::connectex = NULL;
		LPFN_GETACCEPTEXSOCKADDRS tcp::getacceptexsockaddrs = NULL;

		bool init() {
			int f = 1;
			errf(!WSAStartup(MAKEWORD(2,2), &wsaData), f);

			DWORD dwBytes;
			GUID GuidAcceptEx = WSAID_ACCEPTEX;
			GUID GuidGetAcceptEx = WSAID_GETACCEPTEXSOCKADDRS;
			GUID GuidGetConnectEx = WSAID_CONNECTEX;
			
			tcp s;

			s.open();

			errf(WSAIoctl(s.sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
				&GuidAcceptEx, sizeof (GuidAcceptEx), 
				&tcp::acceptex, sizeof (tcp::acceptex), 
				&dwBytes, NULL, NULL) != SOCKET_ERROR, f);

			errf(WSAIoctl(s.sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
				&GuidGetAcceptEx, sizeof (GuidGetAcceptEx), 
				&tcp::getacceptexsockaddrs, sizeof (tcp::getacceptexsockaddrs), 
				&dwBytes, NULL, NULL) != SOCKET_ERROR, f);

			errf(WSAIoctl(s.sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
				&GuidGetConnectEx, sizeof (GuidGetConnectEx), 
				&tcp::connectex, sizeof (tcp::connectex), 
				&dwBytes, NULL, NULL) != SOCKET_ERROR, f);

			s.close();

			return f != 0;
		}

		bool deinit() {
			return err(!WSACleanup()) == 0;
		}

		overlapped::overlapped() : flags(0) {}

		void overlapped::reset() {
			threads::overlapped::reset();
			flags = 0;
		}

		ip::ip() {
			size = sizeof(addr);
			addr.sin_family = AF_INET;
		}

		ip::ip(unsigned short port, char* ipv4) {
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = inet_addr(ipv4);
			addr.sin_port = htons(port);
			size = sizeof(addr);
		}
		
		char* ip::get_ipv4() {
			return inet_ntoa(addr.sin_addr);
		}

		unsigned short ip::get_port() {
			return ntohs(addr.sin_port);
		}

		char* ip::get_local_ipv4() {
			return inet_ntoa(*(struct in_addr *) *(gethostbyname(""))->h_addr_list);
		}

		int ip::size = 0;

		buf::buf(void* data, int len) {
			set(data, len);
		}

		void buf::set(void* data, int len) {
			b.buf = static_cast<char*>(data);
			b.len = len;
		}

		void *buf::get() const {
			return b.buf;
		}

		int buf::get_len() const {
			return b.len;
		}

	}
}

bool augs::threads::iocp::associate(augs::network::udp& io, int key) {
	return err(CreateIoCompletionPort((HANDLE)io.sock, this->cport, key, 0)) != NULL;
}

bool augs::threads::iocp::associate(augs::network::tcp& io, int key) {
	return err(CreateIoCompletionPort((HANDLE)io.sock, this->cport, key, 0)) != NULL;
}