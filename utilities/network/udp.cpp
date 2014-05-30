#pragma once
#include "stdafx.h"
#include "udp.h"
#include "../error/error.h"

namespace augs {
	namespace network {

		udp::udp() : sock(INVALID_SOCKET) {}

		udp::~udp() {
			close();
		}

		bool udp::open() {
			auto res = err(((sock = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED)) != INVALID_SOCKET)) != 0;
			SetFileCompletionNotificationModes((HANDLE)sock, 0);
			return res;
		}

		bool udp::bind(unsigned short port = 0, char* ipv4) {
			addr = ip(port, ipv4);
			return err(::bind(sock, (struct sockaddr*)&addr.addr, sizeof(addr)) == 0) != 0;
		}
		

		int udp::send(overlapped* request) {
			return err((WSASendTo(sock, (LPWSABUF)&request->associated_buffer, 1,
				&request->result, 
				request->flags, 
				(SOCKADDR*)&request->associated_address.addr, 
				request->associated_address.size, &request->overlap, 0) == 0) ? 2 : WSAGetLastError() == WSA_IO_PENDING);
		}
		
		void udp::set_blocking(bool) {
			DWORD nonBlocking = 1;
			err(ioctlsocket(sock,
				FIONBIO,
				&nonBlocking) == 0);
		}
		//int udp::send(const ip& to, const wsabuf* bufs, int bufcnt, overlapped* request) {
		//	return err((WSASendTo(sock, (LPWSABUF)bufs, bufcnt, &request->result, request->flags, (SOCKADDR*)&to.addr, to.size, &request->overlap, 0) == 0) ? 2 : WSAGetLastError() == WSA_IO_PENDING );
		//}

		int udp::recv(overlapped* request) {
			return err((WSARecvFrom(sock, (LPWSABUF) &request->associated_buffer, 1,
				&request->result, 
				&request->flags, 
				(SOCKADDR*) &request->associated_address.addr, 
				&request->associated_address.size, &request->overlap, 0) == 0) ? 2 : WSAGetLastError() == WSA_IO_PENDING);
		}

		//int udp::recv(ip& from, wsabuf* bufs, int bufcnt, overlapped* request) {
		//	return err((WSARecvFrom(sock, (LPWSABUF)bufs, bufcnt, &request->result, &request->flags, (SOCKADDR*)&from.addr, &from.size, &request->overlap, 0) == 0) ? 2 : WSAGetLastError() == WSA_IO_PENDING ); 
		//}
		

		bool udp::send(const ip& to, const wsabuf& b, unsigned long& result, unsigned long flags) {
			return err((WSASendTo(sock, (LPWSABUF)&b, 1, &result, flags, (SOCKADDR*)&to.addr, to.size, 0, 0)) == 0) != 0;
		}
		//
		//bool udp::send(const ip& to, const wsabuf* bufs, int bufcnt, unsigned long& result, unsigned long flags) {
		//	return err((WSASendTo(sock, (LPWSABUF)bufs, bufcnt, &result, flags, (SOCKADDR*)&to.addr, to.size, 0, 0)) == 0) != 0;
		//}
		//
		bool udp::recv(ip& from, wsabuf& b, unsigned long& result, unsigned long& flags) {
			return err((WSARecvFrom(sock, (LPWSABUF)&b, 1, &result, &flags, (SOCKADDR*)&from.addr, &from.size, 0, 0)) == 0) != 0; 
		}
		//
		//bool udp::recv(ip& from, wsabuf* bufs, int bufcnt, unsigned long& result, unsigned long& flags) {
		//	return err((WSARecvFrom(sock, (LPWSABUF)bufs, bufcnt, &result, &flags, (SOCKADDR*)&from.addr, &from.size, 0, 0)) == 0) != 0; 
		//}
		

		bool udp::get_result(overlapped& t) const {
			return err(WSAGetOverlappedResult(sock, &t.overlap, &t.result, false, &t.flags) || WSAGetLastError() == WSA_IO_INCOMPLETE) != 0;
		}

		bool udp::close() {
			bool res = true;
			if(sock != INVALID_SOCKET) {
				res = !closesocket(sock);
				sock = INVALID_SOCKET;
			}
			return err(res) != 0;
		}
	}
}
